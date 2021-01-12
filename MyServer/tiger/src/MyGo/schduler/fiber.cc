

#include "fiber.h"
#include "../../log.h"
#include "../../util.h"



namespace tiger{

  static Logger::ptr g_logger =TIGER_LOG_LOGGER("system");



  void Fiber::run(){
    try{
      
      //执行任务
      m_task();
      m_task = FuncType();  //将function对象进行析构 ---> 内部包含的资源也析构
    }catch(...){
      TIGER_LOG_ERROR(g_logger) << "Fiber Execpt:"<< std::endl << m_backtrace(10);
    }
    m_state = DONE;

    //Fiber执行结束后，切回到Processor进行调度
    //Processor::FiberYield();
  }

  //此函数为Context的启动函数.f解释为Fiber*
  //调用Fiber中的任务
  void Fiber::ContextRun(intptr_t f){
      Fiber* fiber = (Fiber*)f;
      fiber->run();
  }


}
