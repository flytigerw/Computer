#include "processor.h"
#include "scheduler.h"


namespace tiger{



  //某一时刻，在全局范围内正在运行的Fiber
  Fiber::ptr Processor::GetCurrentFiber(){
    auto p = Scheduler::GetCurrentProcessor();
    return p ? p->running_fiber : nullptr;
  }

  //Fiber切回到main fiber ---> Processor的process()
  void Processor::fiber_yield(){
    auto f = GetCurrentFiber();
    f->swap_out();
  }
  
  void Processor::FiberYield(){
    auto p = Scheduler::GetCurrentProcessor();
    if(p)
      p->fiber_yield();
  }


  //对管辖内的Fiber进行调度处理
  void Processor::process(){
    //先要获得一个可运行的Fiber
    while(true){
      runnable.front(running_fiber);
      if(!running_fiber){

      }
    }

  }

}
