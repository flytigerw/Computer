

#ifndef FIBER_H
#define FIBER_H 

#include <ucontext.h>
#include <functional>
#include <memory>
#include "thread.h"


//线程含有一个main协程 ---> 其他协程都必须有main协程创建
namespace sylar{

  class Fiber : public std::enable_shared_from_this<Fiber>{  //类自己可以获取指向自己的shared_pt.继承了该类的对象无法在栈上创建对象 

    public:
      typedef  std::shared_ptr<Fiber>  ptr;

      enum State{
        INIT,
        READY,
        HOLD,
        EXEC,
        TERM,
        EXCPT
      };

    private:
      //默认构造函数私有化 ---> 创建时，必须给出执行的函数地址
      Fiber();
    public:
      Fiber(std::function<void()>cb,size_t stacksize=0,bool use_caller = false);
      ~Fiber();
      //Ex:当前协程执行完任务以后，不销毁协程所占资源，重新給分配任务
      void reset(std::function<void()> cb);
      
      //切换到自己执行
      void swapIn();
      //将执行权让出去
      void swapOut();
      uint64_t getId() const {return m_id;}
      State getState()const {return m_state;}
      void  setState(State s){m_state = s;}
      //弱化版的swapIn和swapOut
      void call();
      void back();

    public:
      static Fiber::ptr GetThis();
      //将f设置为当前协程
      static void SetThis(Fiber* f);
      //让出执行权，将自己转为xx状态
      static void YieldToReady();
      static void YieldToHold();

      static uint64_t TotalFibers();
      static void MainFunc();
      static void CallerMainFunc();
      static uint64_t GetFiberId();
      
     // static MainFunc();
    private:
      //id,栈起点，栈大小，状态，上下文环境，执行的函数
      uint64_t m_id  = 0;
      uint64_t m_stacksize;
      void* m_stack = nullptr;
      State m_state = INIT;
      ucontext_t m_ctx;
      std::function<void()> m_cb;

  };
}


#endif
