#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "hook.h"

namespace sylar{

  static Logger::ptr g_logger = SYLAR_LOG_LOGGER("system");
  
  //当前线程的scheduler
  static thread_local Scheduler* t_scheduler;
  static thread_local Fiber* t_fiber = nullptr;
  Scheduler::Scheduler(size_t threads, bool use_caller ,const std::string& name):m_name(name){
    SYLAR_ASSERT((threads > 0));
    if(use_caller){
        //创建主协程
        sylar::Fiber::GetThis();
        --threads;
        //若在该线程中已经有一个调度器 ---> error
        SYLAR_ASSERT((GetThis() == nullptr));

        t_scheduler = this;
        //创建一个Scheduler时，也创建出root Fiber来运行该scheduler
        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run,this),0,true));
        Thread::setName(m_name);
        t_fiber = m_rootFiber.get();
        //在一个线程中声明调度器，并将该线程纳入该调度器中，那么运行调度器的fiber就不再是线程的main fiber。
        m_rootThreadId = GetThreadId();
        m_threadIds.push_back(m_rootThreadId);
    }else{
      m_rootThreadId = -1;
    }
    m_total_threads = threads;
  }
  Scheduler::~Scheduler(){
    SYLAR_ASSERT(m_stopping);
    if(GetThis() == this)
      t_scheduler = nullptr;
  }

  Scheduler* Scheduler::GetThis(){
        return t_scheduler;
  }
  void Scheduler::setThis(){
    t_scheduler = this;

  }
  Fiber* Scheduler::GetMainFiber(){
        return t_fiber;

  }
  void Scheduler::start(){
    MutexType::LockGuard lock_guard(m_mutex);
    //状态判断
    if(!m_stopping){
        return;
    }
    m_stopping = false;
    SYLAR_ASSERT((m_threads.empty()));
    //创建线程
    m_threads.resize(m_total_threads);
    for(size_t i=0;i<m_total_threads;i++){
      //每一个线程都会运行run
      m_threads[i].reset(new Thread(std::bind(&Scheduler::run,this),m_name+"_"+std::to_string(i)));
      m_threadIds.push_back(m_threads[i]->getId());
    }

#if 0
    if(m_rootFiber)
      m_rootFiber->call();  //主协程swap到run中 ---> 会lock mutex,所以需要提前unlock
      SYLAR_LOG_INFO(g_logger) << "call out";  //若当前scheduler使用了当前线程，那么root fiber的启动不应该在start中 --> 启动会进入到run中，run结束后才会切换回来 --->即start结束.   run()时scheduler的核心逻辑,run结束后，后续操作不再有意义
#endif

  }
  void Scheduler::stop(){
      m_autoStop = true;
      if(m_rootFiber && 
         (m_rootFiber->getState() == Fiber::TERM  ||
         m_rootFiber->getState() == Fiber::INIT) && 
         m_total_threads == 0){

        m_stopping = true;
        if(stopping())
          return;
      }
    ///bool exit_on_this_fiber = false;
    // -1表明没有使用use_caller ---> 可在任意线程stop scheduler
    // 若使用了use_caller，则一点要在创建该scheduler的线程中stop scheduler
    if(m_rootThreadId != -1){
      SYLAR_ASSERT((GetThis() == this));
    }else{
      SYLAR_ASSERT((GetThis() != this));
    }
    m_stopping = true;
    //把所有线程唤醒
    for(size_t i=0; i<m_total_threads;i++)
      notify();
    
    if(m_rootFiber)
      notify();

    //m_rootFiber存在 ----> Scheduler使用了当前线程
    if(m_rootFiber) {
        //while(!stopping()) {
        //    if(m_rootFiber->getState() == Fiber::TERM
        //            || m_rootFiber->getState() == Fiber::EXCEPT) {
        //        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        //        SYLAR_LOG_INFO(g_logger) << " root fiber is term, reset";
        //        t_fiber = m_rootFiber.get();
        //    }
        //    m_rootFiber->call();
        //}
        if(!stopping()) {
        //执行
            m_rootFiber->call();
        }
  }
    std::vector<Thread::ptr> thrs;
    {
        MutexType::LockGuard lock_guard(m_mutex);
        thrs.swap(m_threads);
    }

    for(auto& i : thrs) {
        i->join();
    }
   SYLAR_LOG_INFO(g_logger)  << "Scheduler stoped";
}

  void Scheduler::run(){
    SYLAR_LOG_DEBUG(g_logger) << m_name << " run";
    set_hook_enable(true);
    //设置当前的scheduler
      setThis();
      //没有使用use_caller
      if(GetThreadId() != m_rootThreadId)
        t_fiber = Fiber::GetThis().get();

      SYLAR_LOG_DEBUG(g_logger) << m_name << "create idle fiber";
      Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle,this)));
      SYLAR_LOG_DEBUG(g_logger) << m_name << "create cb fiber";
      Fiber::ptr cb_fiber;

      FiberAndThread ft;
      while(true){
        ft.reset();
        bool notifyThread  = false;
        bool is_active = false;
        {
          MutexType::LockGuard lock_guard(m_mutex);
          auto it = m_fibers.begin();
          //找到一个可运行fiber
          while(it != m_fibers.end()){
            //fiber的threadId 和运行run的线程的id不一致 ----> 要找到归属于该线程的空闲fiber
            if(it->thread_id != -1 && it->thread_id != GetThreadId()){
              ++it;
              //当前线程不能处理，应该通知其他线程来处理
              notifyThread = true;
              continue;
            }
            SYLAR_ASSERT((it->fiber || it->m_cb));

            if(it->fiber && it->fiber->getState() == Fiber::EXEC){
              ++it;
              continue;
            }
            ft = *it;
            m_fibers.erase(it++);
            ++m_active_threads;
            is_active = true;
            break;
          }
           notifyThread |= it != m_fibers.end();
        }

        if(notifyThread)
          notify();

        if(ft.fiber && ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCPT){
            //执行fiber
          SYLAR_LOG_DEBUG(g_logger) << m_name << "ft_fiber swap in";
          ft.fiber->swapIn();
          SYLAR_LOG_DEBUG(g_logger) << m_name << "ft_fiber swap back";
          --m_active_threads;

            //执行完以后的状态检查
          if(ft.fiber->getState() == Fiber::READY)
            schedule(ft.fiber);
          else if(ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCPT)  //让出了执行权
            ft.fiber->setState(Fiber::HOLD);
          ft.reset();
          }else if(ft.m_cb){ //回调的形式
            if(cb_fiber){
              cb_fiber->reset(ft.m_cb);   //用回调函数创建一个fiber
            }else{
              cb_fiber.reset(new Fiber(ft.m_cb));
            }
            SYLAR_LOG_DEBUG(g_logger) << m_name << "cb_fiber reset on new task";
            //执行fiber
            SYLAR_LOG_DEBUG(g_logger) << m_name << "cb_fiber swap in";
            cb_fiber->swapIn();
            SYLAR_LOG_DEBUG(g_logger) << m_name << "cb_fiber swap back";
            --m_active_threads;
            if(cb_fiber->getState() == Fiber::READY){
              schedule(cb_fiber);
              cb_fiber.reset();
            }
            else if(cb_fiber->getState() != Fiber::TERM && cb_fiber->getState() != Fiber::EXCPT){
              cb_fiber->setState(Fiber::HOLD);
              cb_fiber.reset();
            }else  cb_fiber->reset(nullptr); 
          }else{  //空闲的情况
            
            if(is_active) {
                --m_active_threads;
                continue;
            }

            if(idle_fiber->getState() == Fiber::TERM){
              SYLAR_LOG_INFO(g_logger) << "idle fiber term";
              break;
            }
            ++m_idle_threads;
            idle_fiber->swapIn();
            --m_idle_threads;
            if(idle_fiber->getState() != Fiber::TERM && idle_fiber->getState() != Fiber::EXCPT){
              idle_fiber->setState(Fiber::HOLD);
            }
        }
    }
}

void Scheduler::notify(){
  SYLAR_LOG_INFO(g_logger) << "notify";
}
bool Scheduler::stopping(){
  MutexType::LockGuard lock_guard(m_mutex);
  return m_autoStop && m_stopping && m_fibers.empty() && m_active_threads == 0;
}
void Scheduler::idle(){
  
  SYLAR_LOG_INFO(g_logger) << "idle";
  //若idle结束 ---> run结束 
  while(!stopping()) {
      sylar::Fiber::YieldToHold();
}
}

void Scheduler::switchTo(int thread) {
    SYLAR_ASSERT((Scheduler::GetThis() != nullptr));
    if(Scheduler::GetThis() == this) {
        if(thread == -1 || thread == sylar::GetThreadId()) {
            return;
        }
    }
    schedule(Fiber::GetThis(), thread);
    Fiber::YieldToHold();
}

std::ostream& Scheduler::dump(std::ostream& os) {
    os << "[Scheduler name=" << m_name
       << " size=" << m_total_threads
       << " active_count=" << m_active_threads
       << " idle_count=" << m_idle_threads
       << " stopping=" << m_stopping
       << " ]" << std::endl << "    ";
    for(size_t i = 0; i < m_threadIds.size(); ++i) {
        if(i) {
            os << ", ";
        }
        os << m_threadIds[i];
    }
    return os;
}

}
