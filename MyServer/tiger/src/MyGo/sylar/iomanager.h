


#ifndef IOMANAGER_H
#define IOMANAGER_H 


#include "scheduler.h"
#include <memory>
#include "timer.h"

namespace sylar{
  class IOManager:public Scheduler,public TimerManager{

    public:
      typedef std::shared_ptr<IOManager> ptr;
      typedef RWMutex RWMutexType;
      //事件类型
      enum Event{
        NONE = 0x0,
        READ = 0x1,  //EPOLLIN
        WRITE = 0x4  //EPOLLOUT
      };
    private:
      //在epoll_wait中，注册事件是通过添加fd来完成
      //事件的上下文+文件
      struct FdContext{
      typedef Mutex MutexType;
        //在多Scheduler的背景下，我们需要知道该事件在哪个Scheduler上运行
        struct EventContext{
              Scheduler* scheduler = nullptr;       
              Fiber::ptr fiber;
              std::function<void()> cb;   //事件主体
        };

        //传入一个event,得到其context
        EventContext& getContext(Event event);
        void triggerEvent(Event );
        void  resetContext(EventContext& );
          //读写两种事件
          EventContext read;
          EventContext write;
          //事件关联的句柄
          int fd= 0;
          //标识
          Event m_events = NONE;
          MutexType m_mutex;
      }; 

    public:
     IOManager(size_t threads = 1, bool use_caller = true ,const std::string& name = "");
     ~IOManager();
    
     //添加事件:fd,事件类型，回调函数
     //0:success  -1:error
     int addEvent(int fd,Event event,std::function<void()> cb =nullptr);
     bool delEvent(int fd,Event event);
     //取消:找到该事件，使其强制触发执行
     bool cancelEvent(int fd,Event event);
     //把一个fd下的所有事件都取消
     bool cancelAll(int fd);

     static IOManager* GetThis();
    protected:
     void notify() override;
     bool stopping() override;
     bool stopping(uint64_t& time);
     void idle() override;
     void contextResize(size_t );
     void onTimerInsertAtFront() override;
    private:
     //epoll fd
     int m_efd = 0;

     //用管道同步读写进程
     int pfds[2];

     std::atomic<size_t> m_pendingEvent = {0};    //等待待处理的事件数量
    
     RWMutexType m_mutex;
     std::vector<FdContext*> m_fdContexts;


 };
}
#endif
