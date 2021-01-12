#pragma once 

#include <list>
#include <atomic>
#include <condition_variable>
#include <memory>
#include "fiber.h"
#include "../common/refsystem.h"
#include "../common/timer.h"
#include "../datastructrue/incursivestruct.h"


namespace tiger{


  
  class Scheduler;

  //一个Processer对应一个线程
  class Processer{
    friend class Scheduler;


    public:
      typedef TSQueue<Fiber> FiberQueue;
      typedef SList<Fiber> M_List;
      typedef FiberQueue::LockType LockType;
      

      int get_id(){return m_id;}

      //正在运行的Processor和Scheduler
      //声明在Processor class中 ----> 可以直接使用private变量
      static Processer*& GetCurrentProcesser();
      static Scheduler* GetCurrentScheduler();

      //获取正在执行的fiber: --> running_fiber
      static Fiber* GetCurrentFiber();

      //协程切换:static API
      static void FiberYield();


      //私有辅助函数
    private:
      void fiber_yield();

      //可调度的fiber数量:ready+runnable
      size_t runnable_size();

      //添加fiber:新创建的，阻塞后唤醒的
      void add_fiber(Fiber* );
      //将“偷”来的协程add进来
      void add_fiber(M_List&& );

      //将fibers从ready队列转移到run队列中

      bool  add_fibers_to_run_queue();
      //协程队列调度
      void schedule();

      bool is_idle(){return idle;}

      
      //偷协程
      M_List steal(size_t n);

      //fiber销毁
      void gc();

      //阻塞在条件变量上
      void wait_on_conditon();
      void notify_condition();

      // 调度线程打标记, 用于检测阻塞
      void Mark();
      //现在时间 ---> ms表示
      int64_t NowMicrosecond();

    private:
    //数据成员
    int m_id;

    //正在运行的fiber
    Fiber* running_fiber;
    //下一个待运行的fiber ---> pre-fetch
    Fiber* next_fiber;

    // 每轮调度只加有限次数新协程, 防止新协程创建新协程产生死循环
    int add_new_quota = 0;

    // 协程调度次数
    volatile uint64_t schedule_count = 0;

    //协程队列:
    //运行队列
    //等待队列
    //就像队列
    //待回收的fiber队列 ---> 协程运行结束后，会被添加到该队列中
    FiberQueue runnable;
    FiberQueue ready;
    FiberQueue waiting;
    FiberQueue gc_queue;


    //所属的Scheduler
    Scheduler* m_scheduler;


    //若processor没有fiber可调度 ---> 空闲状态 ---> 为其分配fiber
    std::atomic_bool  idle{false};
    //添加fiber到就绪队列后，就可以通知xx将其取出
    //采用条件变量进行同步
    std::condition_variable_any cond;
    bool notified = false;

    //若processor处于非激活态，则不能接受新的fiber加入，但仍然可以强行add_fiber_
    volatile bool m_active;
    
    // 当前正在运行的协程被调度开始的时间戳(Dispatch线程专用)
    volatile int64_t mark_tick = 0;
    volatile uint64_t mark_schedule = 0;



    public:
     //协程挂起相关操作
     //对要挂起的协程记录 每个记录单元为SuspendEntry 
    struct SuspendEntry{

      //weakptr具有检测能力
      WeakPtr<Fiber> m_fiber;
      uint64_t m_id;
      
      SuspendEntry(WeakPtr<Fiber> w,uint64_t id):m_fiber(w),m_id(id){}
      SuspendEntry(){}
      //重载比较符 ---> 可能将SuspendEntry存于map中
      //转为Bbool类型
      explicit operator bool() const {return !!m_fiber;}
      bool operator==(const SuspendEntry& rhs) const {
          return m_fiber == rhs.m_fiber && m_id == rhs.m_id;
          return true;
      }

      bool operator<(const SuspendEntry & rhs) const {
            if (m_id == rhs.m_id)
                return m_fiber < m_fiber;
            return m_id < rhs.m_id;
           return true;
        }
      bool IsExpire() const {
          return Processer::IsSuspendExpire(*this);
      }
    };
    //检测一个SuspendEntry是否有效
    static bool IsSuspendExpire(const SuspendEntry& entry);

    //挂起当前Fiber 
    static SuspendEntry Suspend();
    //挂起并在指定时间后，自动唤醒
    static SuspendEntry Suspend(FastSteadyClock::duration dur);
    static SuspendEntry Suspend(FastSteadyClock::time_point timepoint);

    //协程唤醒
    static bool WakeUp(const SuspendEntry& entry,const std::function<void()>& fctor = nullptr);

    //现在是否出于协程环境中  ---> 判断是否有协程即可
    static bool IsCoroutine();



    private:
    //由Scheduler调用该私有构造函数
    Processer(Scheduler* s,int id):m_id(id),m_scheduler(s){
        //两个队列使用同一个锁
        waiting.set_lock(&runnable.lock_ref());
    }
    //挂起自己旗下的某个fiber
    SuspendEntry suspend_my_fiber(Fiber*);
    //唤醒自己旗下的某个fiber
    bool wakeup_my_fiber(const IncursivePtr<Fiber>& fiber,uint64_t id,const std::function<void()>& fctor);

    //调度线程为其打标记，用于检测阻塞
    void mark();
    //给出当前时间，以ms记录
    int64_t now_ms();
    //若单个fiber执行时长超过预期 ---> 判断为阻塞态
    //阻塞态下，不再为该Processer添加新的fiber,而且Scheduler将fiber分配给其他processor
    bool is_blocking();
  };


}


