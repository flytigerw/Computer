

#include "iomanager.h"

#include <sys/epoll.h>
#include <fcntl.h>
#include "macro.h"
#include <errno.h>

namespace sylar{

  Logger::ptr g_logger = SYLAR_LOG_LOGGER("system");
  IOManager::IOManager(size_t threads, bool use_caller ,const std::string& name):Scheduler(threads,use_caller,name){
    //create epoll fd 
    m_efd = epoll_create(5000);
    SYLAR_ASSERT((m_efd > 0));
    int s = pipe(pfds);
    SYLAR_ASSERT((!s));

    //create epoll event 
    //监听 read end of the pipe
    epoll_event event;
    memset(&event,0,sizeof(event));
    //读触发 +  edge triggered
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = pfds[0];
    //非阻塞式读取
    s = fcntl(pfds[0],F_SETFL,O_NONBLOCK);
    SYLAR_ASSERT((!s));
    
    //将事件添加到epoll中
    s = epoll_ctl(m_efd,EPOLL_CTL_ADD,pfds[0],&event);
    SYLAR_ASSERT((!s));
    
    contextResize(32);
    start();
  }

  IOManager::~IOManager(){
    stop();
    //资源释放
    close(m_efd);
    close(pfds[0]);
    close(pfds[1]);
    
    for(size_t i=0;i<m_fdContexts.size();i++)
      if(m_fdContexts[i])
        delete m_fdContexts[i];
  }
    
  void IOManager::contextResize(size_t s){

    m_fdContexts.resize(s);
    for(size_t i=0;i<m_fdContexts.size();i++)
      if(!m_fdContexts[i]){
        m_fdContexts[i] = new FdContext;
        m_fdContexts[i]->fd = i;  //充当索引

      }
  }

  int IOManager::addEvent(int fd,Event event,std::function<void()> cb){
      FdContext* fd_ctx = nullptr;  //局部变量 ---> 不需要保护
      RWMutexType::ReadLockGuard lock_guard(m_mutex);
      if((int)m_fdContexts.size() > fd){
        fd_ctx = m_fdContexts[fd];    //m_fdContexts为对象的数据，需要保护
        lock_guard.unlock();
      }
      else{
        lock_guard.unlock();
          //需要写保护
        RWMutexType::WriteLockGuard lock_guard2(m_mutex);
        contextResize(fd*1.5);
        fd_ctx = m_fdContexts[fd];
      }

      FdContext::MutexType::LockGuard lock_guard3(fd_ctx->m_mutex);

      //同一fd下，读写事件都应不超过一个
      if(fd_ctx->m_events & event){ 
        SYLAR_LOG_ERROR(g_logger) << "addEvent assert fd = " << fd 
      << "event =" << event 
      << "fd_ctx.m_events " << fd_ctx->m_events;
        SYLAR_ASSERT(!(fd_ctx->m_events & event));
      }

      //判断是添加还是修改事件
      int op = fd_ctx->m_events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;

      epoll_event epevent;
      epevent.events = EPOLLET | fd_ctx->m_events | event;
      epevent.data.ptr = fd_ctx; //value-result

      //add event 
      int s = epoll_ctl(m_efd,op,fd,&epevent);
      if(s) {
        SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_efd << ", "
            << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << s << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
            << (EPOLL_EVENTS)fd_ctx->m_events;
        return -1;
      }
      ++m_pendingEvent;
      fd_ctx->m_events =(Event)(fd_ctx->m_events | event);
      FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
      SYLAR_ASSERT(!event_ctx.scheduler);   
      SYLAR_ASSERT(!event_ctx.fiber);
      SYLAR_ASSERT(!event_ctx.cb);
      //设置上下文
      event_ctx.scheduler = Scheduler::GetThis();
      if(cb)
        event_ctx.cb.swap(cb);
      else{
        event_ctx.fiber = Fiber::GetThis();
        SYLAR_ASSERT((event_ctx.fiber->getState() == Fiber::EXEC));
      }
      return 0;
  }

  bool IOManager::delEvent(int fd,Event event){
      RWMutexType::ReadLockGuard lock_guard1(m_mutex);
      if(fd >=(int)m_fdContexts.size())
        return false;
      FdContext* fd_ctx = m_fdContexts[fd];
      lock_guard1.unlock();

      FdContext::MutexType::LockGuard lock_guard2(fd_ctx->m_mutex);
      //事件不存在
      if(!(fd_ctx->m_events & event))
          return false;

      //通过位运算删除事件
      Event new_event = (Event)(fd_ctx->m_events & ~event);

      int op = fd_ctx->m_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;

      epoll_event epevent;
      epevent.events = EPOLLET | new_event;
      epevent.data.ptr = fd_ctx; 
      int s = epoll_ctl(m_efd,op,fd,&epevent);
      if(s) {
        SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_efd << ", "
            << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << s << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
            << (EPOLL_EVENTS)fd_ctx->m_events;
        return false;
      }
      --m_pendingEvent;
      //重置fdContext
      fd_ctx->m_events = new_event;
      FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
      fd_ctx->resetContext(event_ctx);
      return true;
  }

  bool IOManager::cancelEvent(int fd,Event event){
      RWMutexType::ReadLockGuard lock_guard1(m_mutex);
      if(fd >=(int)m_fdContexts.size())
        return false;
      FdContext* fd_ctx = m_fdContexts[fd];
      lock_guard1.unlock();

      FdContext::MutexType::LockGuard lock_guard2(fd_ctx->m_mutex);
      //事件不存在
      if(!(fd_ctx->m_events & event))
          return false;

      //通过位运算删除事件
      Event new_event = (Event)(fd_ctx->m_events & ~event);

      int op = fd_ctx->m_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;

      epoll_event epevent;
      epevent.events = EPOLLET | new_event;
      epevent.data.ptr = fd_ctx; 
      int s = epoll_ctl(m_efd,op,fd,&epevent);
      if(s) {
        SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_efd << ", "
            << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << s << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
            << (EPOLL_EVENTS)fd_ctx->m_events;
        return false;
      }
      //触发该事件
      fd_ctx->triggerEvent(event);
      --m_pendingEvent;
      return true;

  }
  
  bool IOManager::cancelAll(int fd){
      RWMutexType::ReadLockGuard lock_guard1(m_mutex);
      if(fd >=(int)m_fdContexts.size())
        return false;
      FdContext* fd_ctx = m_fdContexts[fd];
      lock_guard1.unlock();

      FdContext::MutexType::LockGuard lock_guard2(fd_ctx->m_mutex);
      //没有事件
      if(!(fd_ctx->m_events))
          return false;

      int op = fd_ctx->m_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;

      epoll_event epevent;
      epevent.events = 0;
      epevent.data.ptr = fd_ctx; 
      int s = epoll_ctl(m_efd,op,fd,&epevent);
      if(s) {
        SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_efd << ", "
            << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << s << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
            << (EPOLL_EVENTS)fd_ctx->m_events;
        return false;
      }
      
      //触发读写事件
      if(fd_ctx->m_events & READ){
        fd_ctx->triggerEvent(READ);
        --m_pendingEvent;
      }
      if(fd_ctx->m_events & WRITE){
        fd_ctx->triggerEvent(WRITE);
        --m_pendingEvent;
      }
      SYLAR_ASSERT((fd_ctx->m_events == 0));
      return true;
  }

  IOManager* IOManager::GetThis(){
        //从基类转为派生类
        return dynamic_cast<IOManager*>(Scheduler::GetThis());
  }

  //通知:向pipe中写入数据
  void IOManager::notify() {
    if(hasIdleThreads()){  //只有当有空闲线程时，才进行通知
      int s = write(pfds[1],"T",1);
      SYLAR_ASSERT((s == 1));
    }
  }

  bool IOManager::stopping(uint64_t& time) {
    time = getNextTimer();
    return time == ~0ull 
           && Scheduler::stopping()
           && m_pendingEvent == 0;
  }
  bool IOManager::stopping() {
    uint64_t time = 0;
    return stopping(time);
  }

  //当线程空闲时，执行idle
  void IOManager::idle() {
      //协程栈空间较小，不在栈上创建数组，在堆上
      epoll_event* events = new epoll_event[64]();
      //智能指针接管
      std::shared_ptr<epoll_event> shared_events(events,[](epoll_event* e){delete [] e;});
      int s = 0;
      while(true){
      uint64_t time_to_begin  = 0;
        if(stopping(time_to_begin)){
        ///  time_to_begin  = getNextTimer();
          //没有任务的timer
     ///     if(time_to_begin == ~0ull){
          SYLAR_LOG_INFO(g_logger) << "name = " << getName() << " idle stopping exit";
          break;
          }
        //调用epoll_wait进行等待IO事件的发生
        do{
          static const int MAX_TIMEOUT = 5000;        //超时时间
          if(time_to_begin != ~0ull)
            time_to_begin = (int)time_to_begin > MAX_TIMEOUT ? MAX_TIMEOUT : time_to_begin;
          else 
            time_to_begin = MAX_TIMEOUT;
          s = epoll_wait(m_efd,events,64,(int)(time_to_begin));
          if(s < 0 && errno == EINTR){
              //被打断，重来一次
          }else{
            break; //成功 ---> s表明有多少个IO事件发生
          }
        }while(true);
        //执行超时的任务
      std::vector<std::function<void()>> cbs;
      listExpiredCb(cbs);
      if(!cbs.empty()){
        schedule(cbs.begin(),cbs.end());
        cbs.clear();
      }
    
      for(int i=0;i<s;i++){
        epoll_event& event = events[i];
        if(event.data.fd == pfds[0]){  //read end ---> 读出内容，若没有内容，则阻塞等待唤醒
          uint8_t dummy;
          while(read(pfds[0],&dummy,1) == 1);  //由于是ET触发，所以使用while尽可能地读完
          continue;
        }
        //在监听时，我们传入data，现在取出来
        FdContext* fd_ctx =(FdContext*) event.data.ptr;
        //检查监听到的事件的类型
        FdContext::MutexType::LockGuard lock_guard(fd_ctx->m_mutex);
        if(event.events & (EPOLLERR | EPOLLHUP))
          event.events |=   EPOLLIN | EPOLLOUT;         //添加读写事件
        //自定义的事件
        int real_events = NONE;
        if(event.events & EPOLLIN)
          real_events |= READ;
        if(event.events & EPOLLOUT)
          real_events |= WRITE;
        if((fd_ctx->m_events & real_events) == NONE)  //没有共同事件
          continue;
        
        int left_events = (fd_ctx->m_events & ~real_events); //剩余事件
        //继续监听剩余事件
        int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        event.events = EPOLLET | left_events;
        int s2= epoll_ctl(m_efd,op,fd_ctx->fd,&event);
        if(s2) {
          SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_efd << ", "
              << fd_ctx->fd << ", " << (EPOLL_EVENTS)event.events << "):"
              << s << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
              << (EPOLL_EVENTS)fd_ctx->m_events;
          continue;
        }
       if(fd_ctx->m_events & READ){
         fd_ctx->triggerEvent(READ);
         --m_pendingEvent;
       }
       if(fd_ctx->m_events & WRITE){
         fd_ctx->triggerEvent(WRITE);
         --m_pendingEvent;
       }
     }
      //idle让出控制权
      Fiber::ptr cur = Fiber::GetThis();
      auto raw_ptr = cur.get();
      cur.reset();
      raw_ptr->swapOut();
  }
}

  IOManager::FdContext::EventContext& IOManager::FdContext::getContext(Event event){
    switch(event){
      case READ:
        return read;
      case WRITE:
        return write;
      default:
        SYLAR_ASSERT2(false,"getContext");
    }
  }

  //唤醒epoll_wait，重新设置timer
  void IOManager::onTimerInsertAtFront(){
    notify();
  }

  void IOManager::FdContext::resetContext(EventContext& event_ctx){
        event_ctx.scheduler = nullptr;
        event_ctx.fiber.reset();
        event_ctx.cb = nullptr;
  }
  void IOManager::FdContext::triggerEvent(Event event){
      //事件一定要存在
        SYLAR_ASSERT((m_events & event));
        //剩余事件
        m_events = (Event)(m_events & ~event);
        EventContext& ctx = getContext(event);
        if(ctx.cb) {
          ctx.scheduler->schedule(&ctx.cb);
        } else {
          ctx.scheduler->schedule(&ctx.fiber);
        }
       ctx.scheduler = nullptr;
       return;
  }

}
