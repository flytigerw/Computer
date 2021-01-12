


#pragma once

#include <list>
#include <memory>
#include "fiber.h"
#include "../common/lock.h"
#include "../common/TSQueue.h"



namespace tiger{


  class Scheduler;

  //由Scheduler分配线程运行Processor
  //一个Processor管理多个Fiber
  class Processor{


    friend class Scheduler;

    public:
      typedef  TSQueue FiberQueue;

      struct SuspendedFiberEntry{
          std::weak_ptr<Fiber> m_fiber;
          uint64_t m_suspend_id;
      };
    
    private:
      int m_id;
      Fiber::ptr running_fiber;

      //虽然Processor为单线程内的对象.但其可能有多个线程调用成员函数，并发地修改成员数据
      FiberQueue runnable,ready,waiting,gc_queue;
      LFLock run_lock,ready_lock,wait_lock,gc_lock;


    //操作只暴露给Scheduler
    private:

      void process();

      void fiber_yield();

      //static 性质
    public:
      //该Processor中正在运行的Fiber
      static Fiber::ptr GetCurrentFiber();

      static void FiberYield();



      






  };







}
