

#include <dlfcn.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include "../scheduler/scheduler.h"
#include "hookhelper.h"
#include "hook.h"


namespace tiger{
#define HOOK_FUN(XX)\
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)

void hook_init(){
  static bool inited = false;
  if(inited)
    return;
  //比如:sleep_f = (sleep_fun)dlsym(RTLD,"sleep")
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT,#name);
  //批量操作
  HOOK_FUN(XX) 
#undef XX
    inited = true;
}
struct HookIniter{
  HookIniter(){
    hook_init();
  }
};

//以fd封装后FdContex为中心进行操作
bool set_tcp_conn_timeout(int fd,int ms){

  FdContext::ptr fd_ctx = FdCtxManager::GetInstance().get_fd_context(fd);
  if(!fd_ctx)
    return false;
  fd_ctx->set_tcp_conn_timeout(ms);
}

//poll hook : Fiber在调用poll时的处理
//1)pollfd全为负数 ---> 相当于Fiber进行slepp
//2)否则,先进行一次poll的非阻塞调用，查看是否有ready fds 
//3)若没有，则需要将Fiber挂起，将Fiber添加到各个pollfd的相关事件等待队列中，将fd添加到reactor的监听范围内
//4)Fiber挂起返回后
//  a.挂起时间已到 
//  b.有事件到来 ---> 在reactor中的epoll_wait()监听到有事件到来，然后调用trigger() 
//  将到达事件类型写入pollfd.revents，然后返回 ---> 难点:在reactor的epoll_wait()返回后才能拿到事件类型，怎样将事件类型传输到poll()的调用中呢?  ---> 采用共享内存进行通信 ---> 在poll()中分配堆空间，并委托给智能指针防止内存泄露.reactor将事件类型写入到该堆空间即可 ---> poll和reactor之间的桥梁为Fiber ---> poll()会挂起Fiber，Reactor会唤醒Fiber
inline int libgo_poll(struct pollfd* fds,nfds_t nfds,int timeout,bool nonblocking_check){

  Fiber* cur_fiber = Processer::GetCurrentFiber();
  //并没有协程运行，采用原生的poll
  if(!cur_fiber)
    return poll_f(fds,nfds,timeout);

  //本来就是非阻塞调用
  if(timeout == 0)
    return poll_f(fds,nfds,timeout);

  //查看负数fd的个数
  nfds_t negative_fd_n = 0;
  for (nfds_t i = 0; i < nfds; ++i)
       if (fds[i].fd < 0)
          ++ negative_fd_n;

  //若全都为负数，则就相当于将fiber进行sleep
  //fiber的sleep ---> 在挂起给定时间后被唤醒

  if(nfds == negative_fd_n){
    if(timeout > 0){
      Processer::Suspend(std::chrono::microseconds(timeout));
      Processer::FiberYield();  //切回到主协程进行调度
    }
  }

  //唤醒以后
  if(nonblocking_check){
    //先执行一次非阻塞的poll.若有ready fd,则直接返回
    int s = poll_f(fds,nfds,0);
    if(s != 0)
      return s;
  }

  //还是没有ready fd,则需要将Fiber挂起，并将FiberEntry添加到fd的事件等待队列中
  short int * arr_events = new short int[nfds];
  memset(arr_events, 0, sizeof(short int) * nfds);
  std::shared_ptr<short int> revents(arr_events, [](short int* p){ delete[] p; });

  //对fiber进行挂起 --> Suspend只是将其添加到processor的阻塞队列中,并不会引起协程切换
  Processer::SuspendEntry entry;
  if (timeout > 0)
      entry = Processer::Suspend(std::chrono::milliseconds(timeout));
  else
     entry = Processer::Suspend();

  //遍历fds,将该Fiber添加到fd的各个等待事件队列(events)中
  //一个Fiber可以调用poll()监听多个fd上的IO
  bool added = false;
  for(nfds_t i = 0;i<nfds;++i){
    pollfd& pfd = fds[i];
    pfd.revents = 0;      //clear revents 
    if(pfd.fd < 0)        //ignore
      continue;

    if(!Reactor::Select(pfd.fd)                                   //得到该fd所属的Reactor
        .add(pfd,pfd.events,Reactor::Entry(entry,revents,i))){    //1)将该Fiber添加到该fd的事件队列中
                                                                  //2)将fd添加到reactor的监听范围中

      // bad file descriptor
      arr_events[i] = POLLNVAL;
      continue;
    }
     added = true;
  }

  // 全部fd都无法加入epoll ---> Bad fd ---> pollfd.revents填入POLLNVAL ---> 在trigger时会唤醒所有事件等待队列
  if(!added){
    for(nfds_t i =0;i<nfds;++i)
      fds[i].revents = arr_events[i];
    errno = 0;
    Processer::WakeUp(entry);   //解除Fiber的挂起
    Processer::FiberYield();    //切回到主协程
    return nfds;
  }

  //前面已经调用了Suspend(entry),现在切回到主协程处理BLOCK状态
  Processer::FiberYield();

  //Fiber被唤醒后 
  //唤醒时机:
  //1)在EpollReactor中，epoll_wait()监听返回后会对ready fds进行处理 ---> 根据到来的事件唤醒fd的相关事件等待队列 --> 而trigger会将到来的事件类型event_type写入ReactorEvent::Entry的shared_ptr<short int> m_arr_event_types --->revents,arr_events,m_arr_event_types三者都指向同一片堆内存
  //2)超时
  int n = 0;
  for (nfds_t i = 0; i < nfds; ++i) {
      fds[i].revents = arr_events[i];
      if (fds[i].revents)    //poll()成功后，各个pollfd的revents必须要有值
        ++n;
   }
  errno = 0;
  return n;
}


}

using namespace tiger;
//IO通用hook
//比如read的调用 read_write_mode(fd, read_f, "read", POLLIN, SO_RCVTIMEO, count, buf, count);
//F表示原本的函数
template<class F,class ...Args>
static ssize_t read_write_mode(int fd,F f,const char* hook_f_name,
                                short int event_type,int timeout_type,
                                ssize_t buf,Args&& ...args){

  Fiber* cur_fiber = Processer::GetCurrentFiber();
  //并没有协程运行，采用原生的函数
  if(!cur_fiber)
    return f(fd,std::forward<Args>(args)...);

  //得到FdContex,针对其属性进行操作
  FdContext::ptr fd_ctx = FdCtxManager::GetInstance().get_fd_context(fd);
  //fd设置了O_NONBLOCK --->调用原生版本 ---> 非阻塞式
  if(!fd_ctx || fd_ctx->is_nonblocking())
    return f(fd,std::forward<Args>(args)...);

  //阻塞式
  long socket_timeout = fd_ctx->get_socket_timeout(timeout_type);
  int poll_timeout = (socket_timeout == 0) ? -1 : (socket_timeout < 1000 ? 1 : socket_timeout / 1000); //单位换算
  
  //调用hook后poll
  struct pollfd fds;
  fds.fd = fd;
  fds.events = event_type;
  fds.revents = 0;

INTR:
  int s= libgo_poll(&fds,1,poll_timeout,true);
  if(-1 == s){
    if(errno == EINTR)
      goto INTR;        //信号中断 //采用pollf?
  }else if(0 == s){    //等待超时，置errno = EAGAIN
    errno = EAGAIN;
    return -1;
  }

  //进行到此，说明fd上的IO已经就绪 ---> 调用原生版本即可
INTR2:
  ssize_t res = f(fd,std::forward<Args>(args)...);
  if(res == -1){
    if(errno == EINTR)
      goto INTR2;
    return -1;
  }
  return res;
}


//自定义的hook 函数
//要用gcc编译
extern "C"{
#define XX(name)\
  name ## _fun name ## _f = nullptr;
  HOOK_FUN(XX)
#undef XX



}
