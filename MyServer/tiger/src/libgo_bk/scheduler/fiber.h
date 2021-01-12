#pragma once 
#include <functional>
#include "../context/context.h"
#include "../../base.h"
#include "../common/refsystem.h"
#include "../datastructrue/incursivestruct.h"
#include "../common/anys.h"


namespace tiger{
  //Processor调度维护一个Fiber队列
  //队列底层为双向链表
  //链表节点
  
  enum class FiberState{
    RUNNABLE,
    BLOCK,
    DONE,
  };

  typedef std::function<void()> FuncType;
  class Processer;

  
  //SharedRefCounter : Noncopyble
 struct Fiber :public SharedRefCounter, public ListNode{ 

    
  Fiber(FuncType const& task, std::size_t stack_size)
        :m_ctx(&Fiber::ContextRun, (intptr_t)this, stack_size), 
         m_task(task){}

//    DebugPrint(dbg_task, "task(%s) construct. this=%p", DebugInfo(), this);

    //数据成员:id,state,自己属于哪一个processor
    uint64_t m_id;
    Context  m_ctx;
    FuncType m_task;         //传给Context
    FiberState m_state = FiberState::RUNNABLE;
    Processer* m_processer;

    //记录自己yield的次数
    uint64_t yield_count = 0;

    std::atomic<uint64_t> suspend_id {0};

    //上下文的切换
    void swap_in(){
      m_ctx.swap_in();
    }

    void swap_to(Fiber* f){
      m_ctx.swap_to(f->m_ctx);
    }
    void swap_out(){
      m_ctx.swap_out();
    }

    private:
     void run();
     //Context启动函数,由Fiber为其配套
     static void ContextRun(intptr_t task);
  };

struct FiberGroupKey {};
typedef Anys<FiberGroupKey> FiberAnys;











}
