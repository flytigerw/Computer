
#ifndef TIMER_H
#define TIMER_H 


#include <memory>
#include "thread.h"
#include <set>
#include <vector>


namespace sylar{
  class TimerManager;
  
  class Timer : public std::enable_shared_from_this<Timer>{

    friend class TimerManager;
    public:
      typedef std::shared_ptr<Timer> ptr;

      bool cancel();
      bool refresh();
      bool reset(uint64_t ms,bool from_now);
    private:
      //构造函数私有化 ---> 通过友元timermanger创建timer
      Timer(uint64_t ms,std::function<void()> cb,bool recurring,TimerManager*);
      Timer(uint64_t next):m_next(next){}
      
    private:
      //是否为循环定时器
      uint64_t m_next;           //精确的执行时间
      uint64_t m_ms;             //执行周期
      bool m_recurring;
      TimerManager* m_manager;
      //定时器需要执行的任务
      std::function<void()> m_cb;
    private:
      struct Comparator{
        bool operator()(const Timer::ptr& lhs,const Timer::ptr& rhs) const;
      };
  };

  class TimerManager{
      friend class Timer;
    public:
      typedef RWMutex RWMutexType;
    public:
      TimerManager();
      Timer::ptr addTimer(uint64_t ms,std::function<void()>cb,bool recurring = false);
      void addTimer(Timer::ptr val,RWMutexType::WriteLockGuard& lock_guard);
      //条件定时器。只有当条件满足时才会被触发 ---> 可能条件发生改变，此时没有必要再触发Timer
      Timer::ptr addConditionTimer(uint64_t ms,std::function<void()> cb, 
                                   std::weak_ptr<void> weak_cond,bool recurring); //以智能指针作为条件，利用其引用计数
      //获取定时器还有多少时间开始执行，比如0，表明应该立刻执行定时器定时的任务
      uint64_t getNextTimer();

      //返回出 超时定时器的的cb ---> 让scheduler执行
      void listExpiredCb(std::vector<std::function<void()>>& cbs);
      //检测系统时间是否发生更改
      bool detectClockChange(uint64_t now_ms);
    protected:
      //IO event陷入了epoll_wait
      //此时另外一个线程add timer,而且timer的时间变小，那么就应该通知IO event对等待的时间进行修改 --> 太大了
      virtual void onTimerInsertAtFront() = 0;
      bool hasTimer();
    private:
      RWMutex m_mutex;
      //采用有序容器容纳timer
      std::set<Timer::ptr,Timer::Comparator> m_timers;

      bool m_notify = false;
      uint64_t previousTime = 0;
  };
}

#endif
