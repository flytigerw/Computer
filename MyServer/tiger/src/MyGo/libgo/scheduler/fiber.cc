#include "fiber.h"
#include "processor.h"

namespace tiger{


  void Fiber::run(){
    //先不考虑:Listener，options，debug
    try{
      //执行任务
      this->m_task();
      //让协程function对象的析构也在协程中执行
      this->m_task = FuncType();
    }catch(...){
      //异常处理
    }
    m_state = FiberState::DONE;
    //执行结束后，由Processor重新调度
    Processer::FiberYield();
   // Processer::StaticCoYield();
  }

  void Fiber::ContextRun(intptr_t f){
    Fiber* fiber = (Fiber*) f;
    fiber->run();
  }








}
