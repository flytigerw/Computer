
#include "hook.h"
#include "iomanager.h"
#include "fd_manager.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include "config.h"



sylar::Logger::ptr g_logger = SYLAR_LOG_LOGGER("system");
namespace sylar{
  //在配置文件中查找timeout
  static sylar::ConfigVar<int>::ptr g_tcp_connect_timeout =
    sylar::Config::LookUp("tcp.connect.timeout", 5000, "tcp connect timeout");


  //实现线程级别的hook 
  //采用线程局部变量来表明该线程是否hook
  static thread_local bool t_hook_enable = false;

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

  static uint64_t s_connect_timeout = -1;
  void hook_init(){
    static bool is_inited = false;
    if(is_inited)
      return;
    //在动态库中查找该函数的地址
    //比如name为slepp
    //将其转为sleep_fun
    //赋给变量sleep_f
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT,#name);
    HOOK_FUN(XX);
#undef XX
  }
  bool is_hook_enable(){
    return true;

  }
  void set_hook_enable(bool flag){
    t_hook_enable = flag;
  }

  //在main函数之前完成hook的init
  struct HookIniter{
    HookIniter(){
      hook_init();
      s_connect_timeout = g_tcp_connect_timeout->getValue();
      //配置变更的listener
      g_tcp_connect_timeout->addListener([](const int& old_value, const int& new_value){
              SYLAR_LOG_INFO(g_logger) << "tcp connect timeout changed from "
                                       << old_value << " to " << new_value;
              s_connect_timeout = new_value;
      });
    }
  };

  //store some info
  struct timer_info{
    int cancelled = 0;
  };
  static HookIniter __hook_init;

    //IO函数的模板
    template<class OriginFun,class ...Args>
      static ssize_t do_io(int fd,OriginFun func,const char* hook_fun_name,uint32_t event,int type,Args&&...args){
        if(!t_hook_enable)
          return func(fd,std::forward<Args>(args)...);  //调用原本的函数

        FdCtx::ptr fd_ctx = FdMgr::GetInstance()->get(fd);
        if(!fd_ctx)                                     //不存在也就不是socket,调用原来的函数
          return func(fd,std::forward<Args>(args)...);  
        if(fd_ctx->isClosed()){
          errno = EBADF;
          return -1;
        }

        if(!fd_ctx->isSocket() || fd_ctx->isUserNonblock())
          return func(fd,std::forward<Args>(args)...);  

        uint64_t time_out = fd_ctx->getTimeout(type);

        std::shared_ptr<timer_info> tinfo(new timer_info);  //条件 ---> 在其他处通过weak_ptr进行判断

retry:
        ssize_t n =func(fd,std::forward<Args>(args)...);  //调用原本的函数,读取数据
        while(n == -1 && errno == EINTR)
            func(fd,std::forward<Args>(args)...);         //被信号中断，重新再来
        if(n == -1 && errno == EAGAIN) {                //非阻塞IO,读取数据 ---> 数据尚未到来 ----> 按自己的方式阻塞
          IOManager* iom = IOManager::GetThis();
          Timer::ptr timer;
          std::weak_ptr<timer_info> winfo(tinfo);

          if(time_out !=(uint64_t)-1)  //含有超时时间  ---> 为Timer设置超时时间
            timer = iom->addConditionTimer(time_out,[winfo,fd,iom,event](){
                auto t = winfo.lock();
                if(!t || t->cancelled){ //定时器已经失效    
                  return;
                }
                t->cancelled = ETIMEDOUT;     //condition
                iom->cancelEvent(fd,(IOManager::Event)(event));   //强制执行
                },winfo,false);
            int c =0;  //计数器
            uint64_t now = 0;
            //添加事件
            int rt = iom->addEvent(fd,(IOManager::Event)(event));
            if(rt){    //非0 ---> error  添加失败
              if(c){
               SYLAR_LOG_ERROR(g_logger) << hook_fun_name << " addEvent (" << fd << ", " << event << ")"
                                         <<"retry c= " << c
                                         << "used = " << GetCurrentUS() - now;
              }
              if(timer)
                timer->cancel();
              return -1;
            }else{  //添加成功
              Fiber::YieldToHold();
              //唤醒回来后
              if(timer)
                timer->cancel();
              if(tinfo->cancelled){   //超时
                errno = tinfo->cancelled;
                return -1;
            }
              //并非超时 ---> 有数据到达 ----> 重新调用函数
              goto retry;
           }
        }  
        return n;
      }

  extern "C"{
    //声明一个变量:sleep_f
#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX


    //-------------------------------------------------slepp APIs --------------------------------------
    unsigned int sleep(unsigned int seconds){
      if(!t_hook_enable)
        return sleep_f(seconds);  //直接使用原函数
      //以下为自定义的实现
      Fiber::ptr fiber = Fiber::GetThis();
      IOManager* iom = IOManager::GetThis();
      //在指定时间后，将该fiber进行schedule
      //即:原始sleep会使得线程阻塞 ---> 所有fiber阻塞
      //现在的sleep只会"阻塞"fiber,fiber会陷入epoll_wait
      iom->addTimer(seconds*1000,[iom,fiber](){
          iom->schedule(fiber);
          });
      //自己先陷入hold状态，待会进行schedule
      Fiber::YieldToHold();
      return 0;
    }

    int usleep(useconds_t usec){
      if(!t_hook_enable)
        return usleep_f(usec);  
      Fiber::ptr fiber = Fiber::GetThis();
      IOManager* iom = IOManager::GetThis();
      //bind绑定模板成员函数
      iom->addTimer(usec/1000,std::bind((void(Scheduler::*)(Fiber::ptr,int thread))&IOManager::schedule
                                      ,iom,fiber,-1));
      Fiber::YieldToHold();
      return 0;
    }

    int nanosleep(const struct timespec* req,struct timespec* rem){
      if(!t_hook_enable)
        return  nanosleep_f(req,rem);
      int ms = req->tv_sec*1000 + req->tv_nsec/1000 / 1000;
      Fiber::ptr fiber = Fiber::GetThis();
      IOManager* iom = IOManager::GetThis();
      iom->addTimer(ms,std::bind((void(Scheduler::*)(Fiber::ptr,int thread))&IOManager::schedule
                                      ,iom,fiber,-1));
      Fiber::YieldToHold();
      return 0;
    }

    //--------------------------------------------------SOCKET APIs ----------------------------------------------
    int socket(int domain,int type,int protocol){
        if(!t_hook_enable)
          return socket_f(domain,type,protocol);
        //do something else
        int fd = socket_f(domain, type, protocol);
        if(fd == -1) {
           return fd;
        }
        FdMgr::GetInstance()->get(fd, true);
        return fd;
    }


   int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
        //accept 返回新的socket fd
        int fd = do_io(s, accept_f, "accept", sylar::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
        if(fd >= 0) {
          sylar::FdMgr::GetInstance()->get(fd, true);
        }
        return fd;
    }
   int close(int fd){
     if(!sylar::t_hook_enable) {
         return close_f(fd);
     }
     //需要做的额外事件:触发所有与fd相关联的事件
     FdCtx::ptr fd_ctx = FdMgr::GetInstance()->get(fd);
     if(fd_ctx){
       auto iom = IOManager::GetThis();
       if(iom)
         iom->cancelAll(fd);
       FdMgr::GetInstance()->del(fd);
     }
     return close_f(fd);
   }


   //类似do_io
  int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms) {
      if(!sylar::t_hook_enable) {
          return connect_f(fd, addr, addrlen);
      }
      sylar::FdCtx::ptr ctx = sylar::FdMgr::GetInstance()->get(fd);
      if(!ctx || ctx->isClosed()) {
          errno = EBADF;
          return -1;
      }
      if(!ctx->isSocket()) 
          return connect_f(fd, addr, addrlen);

      //用户自己设置了非阻塞方式，直接调用原函数即可
      if(ctx->isUserNonblock()) 
          return connect_f(fd, addr, addrlen);

      //非阻塞调用
      int n = connect_f(fd, addr, addrlen);
      if(n == 0) 
        return 0;
      //其他错误
      else if(n != -1 || errno != EINPROGRESS) 
        return n;
      
      //没有连接成功
      sylar::IOManager* iom = sylar::IOManager::GetThis();
      sylar::Timer::ptr timer;
      std::shared_ptr<timer_info> tinfo(new timer_info);
      std::weak_ptr<timer_info> winfo(tinfo);

      //设置条件定时器
      if(timeout_ms != (uint64_t)-1) {
          timer = iom->addConditionTimer(timeout_ms, [winfo, fd, iom]() {
                  auto t = winfo.lock();
                  if(!t || t->cancelled) {
                      return;
                  }
                   t->cancelled = ETIMEDOUT;
                  iom->cancelEvent(fd, sylar::IOManager::WRITE);
          }, winfo,false);
      }
      //添加监听event
      int rt = iom->addEvent(fd, sylar::IOManager::WRITE);
      if(rt == 0) {
          sylar::Fiber::YieldToHold();
          //被唤醒后:超时唤醒 或者 数据到来
          if(timer) 
              timer->cancel();
          if(tinfo->cancelled) {
              errno = tinfo->cancelled;
              return -1;
          }
      } else {
          if(timer) 
              timer->cancel();
          SYLAR_LOG_ERROR(g_logger) << "connect addEvent(" << fd << ", WRITE) error";
      }

      int error = 0;
      socklen_t len = sizeof(int);
      if(-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)) 
          return -1;
      if(!error) 
          return 0;
      else {
         errno = error;
         return -1;
      }
  }


   int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
      return connect_with_timeout(sockfd, addr, addrlen,s_connect_timeout);
   }










   //--------------------------------------------------------other-------------------------------------------

   int fcntl(int fd, int cmd, ... /* arg */ ) {
     //可变参数cmd
      va_list va;
      va_start(va, cmd);
      switch(cmd) {
          case F_SETFL:
              {
                //取出可变参数中的剩余部分:int类型
                  int arg = va_arg(va, int);
                  va_end(va);
                  sylar::FdCtx::ptr ctx = sylar::FdMgr::GetInstance()->get(fd);
                  //只关心:将scoket文件设为NONBLOCK
                
                  if(!ctx || ctx->isClosed() || !ctx->isSocket()) 
                      return fcntl_f(fd, cmd, arg);
                  //判断是否需要设置为user nonblock
                  ctx->setUserNonblock(arg & O_NONBLOCK);
                  if(ctx->isSysNonblock()) 
                      arg |= O_NONBLOCK;
                  else 
                      arg &= ~O_NONBLOCK;
                  return fcntl_f(fd, cmd, arg);
              }
               break;
          case F_GETFL:
              {
                  va_end(va);
                  int arg = fcntl_f(fd, cmd);
                  sylar::FdCtx::ptr ctx = sylar::FdMgr::GetInstance()->get(fd);
                  if(!ctx || ctx->isClosed() || !ctx->isSocket()) {
                      return arg;
                  }
                  if(ctx->isUserNonblock()) 
                      return arg | O_NONBLOCK;
                  else 
                      return arg & ~O_NONBLOCK;
              }
              break;
              //we don't care:直接执行原函数，返回即可
          case F_DUPFD:
          case F_DUPFD_CLOEXEC:
          case F_SETFD:
          case F_SETOWN:
          case F_SETSIG:
          case F_SETLEASE:
          case F_NOTIFY:
#ifdef F_SETPIPE_SZ
          case F_SETPIPE_SZ:
#endif
              {
                  int arg = va_arg(va, int);
                  va_end(va);
                  return fcntl_f(fd, cmd, arg); 
              }
              break;
          case F_GETFD:
          case F_GETOWN:
          case F_GETSIG:
          case F_GETLEASE:
#ifdef F_GETPIPE_SZ
          case F_GETPIPE_SZ:
#endif
              {
                  va_end(va);
                  return fcntl_f(fd, cmd);
              }
              break;
          case F_SETLK:
          case F_SETLKW:
          case F_GETLK:
              {
                  struct flock* arg = va_arg(va, struct flock*);
                  va_end(va);
                  return fcntl_f(fd, cmd, arg);
              }
              break;
          case F_GETOWN_EX:
          case F_SETOWN_EX:
              {
                  struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                  va_end(va);
                  return fcntl_f(fd, cmd, arg);
              }
              break;
          default:
              va_end(va);
              return fcntl_f(fd, cmd);
    }
}

  int ioctl(int d, unsigned long int request, ...) {
      va_list va;
      va_start(va, request);
      void* arg = va_arg(va, void*);
      va_end(va);

      //判断是否需要设置user_nonblock，若需要则在FdMgr中记录
      if(FIONBIO == request) {
          bool user_nonblock = !!*(int*)arg;  //!!转为bool型
          sylar::FdCtx::ptr ctx = sylar::FdMgr::GetInstance()->get(d);
          if(!ctx || ctx->isClosed() || !ctx->isSocket()) {
              return ioctl_f(d, request, arg);
          }
          ctx->setUserNonblock(user_nonblock);
      }
      return ioctl_f(d, request, arg);
   }
//获取socket的相关option
  int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
      return getsockopt_f(sockfd, level, optname, optval, optlen);
  }

//设置socket超时相关的内容
  int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
      if(!sylar::t_hook_enable) {
          return setsockopt_f(sockfd, level, optname, optval, optlen);
      }
      if(level == SOL_SOCKET) {
          if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
              sylar::FdCtx::ptr ctx = sylar::FdMgr::GetInstance()->get(sockfd);
              if(ctx) {
                  const timeval* v = (const timeval*)optval;
                  ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
              }
          }
      }  
    return setsockopt_f(sockfd, level, optname, optval, optlen);
  }
//-----------------------------------------------IO APIs
  ssize_t read(int fd, void *buf, size_t count) {
      return do_io(fd, read_f, "read", sylar::IOManager::READ, SO_RCVTIMEO, buf, count);
  }

  ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
      return do_io(fd, readv_f, "readv", sylar::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
  }

  ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
      return do_io(sockfd, recv_f, "recv", sylar::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
  }

  ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
      return do_io(sockfd, recvfrom_f, "recvfrom", sylar::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
  }

  ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
      return do_io(sockfd, recvmsg_f, "recvmsg", sylar::IOManager::READ, SO_RCVTIMEO, msg, flags);
  }

  ssize_t write(int fd, const void *buf, size_t count) {
      return do_io(fd, write_f, "write", sylar::IOManager::WRITE, SO_SNDTIMEO, buf, count);
  }

  ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
      return do_io(fd, writev_f, "writev", sylar::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
  }

  ssize_t send(int s, const void *msg, size_t len, int flags) {
      return do_io(s, send_f, "send", sylar::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
  }

  ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
      return do_io(s, sendto_f, "sendto", sylar::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
  }

  ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
      return do_io(s, sendmsg_f, "sendmsg", sylar::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
  }
  

  

  }
}
