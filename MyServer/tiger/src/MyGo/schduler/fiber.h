#pragma once 

#include <memory>
#include "../context/context.h"

namespace tiger{


  class Processor;

  class Fiber : public std::enable_shared_from_this<Fiber>{

    public:
      enum State{
        RUNNABLE,
        BLOCK,
        DONE,
      };
      typedef std::shared_ptr<Fiber> ptr;
      typedef std::function<void()> FuncType;

    private:
      Context m_ctx;
      FuncType m_task;
      uint64_t m_id;
      State m_state = RUNNABLE;

    public:
      //Fiber构造:会创建底层Context对象.
      //Context对象需要启动函数，栈相关参数
      Fiber(const FuncType& task,std::size_t stack_size)
              :m_ctx(&Fiber::ContextRun,(intptr_t)this,stack_size)   
              ,m_task(task){}

    public:
      void swap_in(){
        m_ctx.swap_in();
      }

      void swap_out(){
        m_ctx.swap_out();
      }

      //私有函数
    private:
      void run();
      static void ContextRun(intptr_t fiber);

  };



}
