

#ifndef M_FIBER_H
#define M_FIBER_H 

#include <ucontext.h>
#include <functional>
#include <memory>
#include "thread.h"

namespace tiger{

  class Fiber : public std::enable_shared_from_this<Fiber>{
    public:
      typedef std::shared_ptr<Fiber>  ptr;

      Fiber(std::function<void()> cb,const std::string& name="",size_t stacksize = 0);
      ~Fiber();

      enum State{
        INIT,
        READY,
        EXEC,
        HOLD,
        TERM,
        EXCPT
      };


      static uint64_t GetCurFiberId();
      static void SetCurFiber(std::shared_ptr<Fiber>);
      static std::shared_ptr<Fiber> GetCurFiber();

      //切回到主协程
      static void swap();
      //fiber主动放弃 ----> 切回到主协程中
      static void yield();
    
      static void run();
      //主协程的任务
      static void main_run();
    private:
      Fiber();
    
    private:

      ucontext_t m_ctx;
      std::function<void()> m_cb;
      void* m_stack;
      uint64_t m_id;
      std::string m_name;
      uint32_t m_stacksize;

      State m_state = INIT;
      
  };


}

#endif
