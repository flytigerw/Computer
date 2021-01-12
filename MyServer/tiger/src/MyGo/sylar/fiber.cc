
#include <atomic>
#include "fiber.h"
#include "config.h"
#include "scheduler.h"
#include "macro.h"


namespace sylar{

  static Logger::ptr g_logger = SYLAR_LOG_LOGGER("system");
  //全局性的计数数据 ---> 采用atomic,不再加锁
  static std::atomic<int> s_fiber_id{0};
  static std::atomic<int> s_fiber_count{0};
  //在堆上分配栈 
  class MallocStackAllocator{
    public:
    static void* Alloc(size_t size){
      return malloc(size);
    }
    static void Delloc(void* vp,size_t stacksize){
      free(vp);
    }
  };
  using StackAllocator = MallocStackAllocator;

  //用一个线程局部变量表示线程中的协程
  static thread_local Fiber* t_fiber = nullptr;
  //main协程
  static thread_local Fiber::ptr t_main_fiber = nullptr;
  //栈的配置项
  static ConfigVar<uint32_t>::ptr g_fiber_stack_size = 
      Config::LookUp<uint32_t>("fiber.stack.size",1024*1024,"fiber stack size");
  
  //私有化的默认构造函数 ---> 第一个协程使用 --> 主协程
  Fiber::Fiber(){
      m_state = EXEC;
      SetThis(this);
      //main协程获得线程的上下文
      if(getcontext(&m_ctx)){
        SYLAR_ASSERT2(false,"getcontext");
      }
      ++s_fiber_count;
    SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber main";
  }

  Fiber::Fiber(std::function<void()>cb,size_t stacksize,bool use_caller):m_id(++s_fiber_id),m_cb(cb){
    ++s_fiber_count;
    //若为0，则从配置中读取
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();
    m_stack = StackAllocator::Alloc(m_stacksize);
    if(getcontext(&m_ctx)){
       SYLAR_ASSERT2(false,"getcontext");
    }
    m_ctx.uc_link = nullptr;  //当前上下文执行结束后会回答m_link指向的上下文
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    if(!use_caller)
      //用设置好的上下文去执行函数
      makecontext(&m_ctx,&Fiber::MainFunc,0);
    else 
      makecontext(&m_ctx,&Fiber::CallerMainFunc,0);
    
    SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
  }

  Fiber::~Fiber(){
    --s_fiber_count;
    //栈资源回收
    if(m_stack){
      SYLAR_ASSERT((m_state == INIT || m_state == EXCPT || m_state == TERM)); //若析构时，协程状态不处于这三种 ---> assert 
      StackAllocator::Delloc(m_stack,m_stacksize);
    }else{  //主协程没有栈
      SYLAR_ASSERT((!m_cb  && m_state == EXEC));   //根据main fiber的性质进一步判断是否为main fiber 
      if(t_fiber == this)
        SetThis(nullptr);
    }
    SYLAR_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id
                              << " total =" << s_fiber_count;
  }

  void Fiber::reset(std::function<void()> cb){
      //为fiber重置任务，要满足条件:自己有stack，状态为TERM或者为INIT
      SYLAR_ASSERT(m_stack);
      SYLAR_ASSERT((m_state == TERM || m_state == INIT || m_state == EXCPT));  
      m_cb = cb;
      if(getcontext(&m_ctx)){
         SYLAR_ASSERT2(false,"getcontext");
      }
      m_ctx.uc_link = nullptr;  
      m_ctx.uc_stack.ss_sp = m_stack;
      m_ctx.uc_stack.ss_size = m_stacksize;
      //用设置好的上下文去执行函数
      makecontext(&m_ctx,&Fiber::MainFunc,0);
      m_state = INIT;
  }
      
  //从main fiber切换到调用者运行
  void Fiber::swapIn(){
      SetThis(this);
      SYLAR_ASSERT((m_state != EXEC));
      m_state = EXEC;
      if(swapcontext(&(Scheduler::GetMainFiber()->m_ctx),&m_ctx)){
              SYLAR_ASSERT2(false,"swapcontext");
      }

 //      if(swapcontext(&(t_main_fiber->m_ctx),&m_ctx)){
 //             SYLAR_ASSERT2(false,"swapcontext");
 //     }
 

  }
  //将当前协程放到后台，切换到main fiber
  void Fiber::swapOut(){
#if 0
    if(this != Scheduler::GetMainFiber()){
       SetThis(Scheduler::GetMainFiber());  
       if(swapcontext(&m_ctx,&(Scheduler::GetMainFiber()->m_ctx)))
            SYLAR_ASSERT2(false,"swapcontext");
    }else{
       SetThis(t_main_fiber.get());  
       if(swapcontext(&m_ctx,&t_main_fiber->m_ctx))
            SYLAR_ASSERT2(false,"swapcontext");
    }
#endif 
     SetThis(Scheduler::GetMainFiber());  
     if(swapcontext(&m_ctx,&(Scheduler::GetMainFiber()->m_ctx)))
          SYLAR_ASSERT2(false,"swapcontext");
  }
  Fiber::ptr Fiber::GetThis(){
      if(t_fiber)
        return t_fiber->shared_from_this();
      //若当前协程不存在，则创建主协程
      Fiber::ptr main_fiber(new Fiber);
      SYLAR_ASSERT((t_fiber == main_fiber.get()));
      t_main_fiber = main_fiber;
      return t_fiber->shared_from_this();
  }
  void  Fiber::SetThis(Fiber* f){
        t_fiber = f;
  }

  void Fiber::call(){
     SetThis(this);
     m_state  = EXEC;
     if(swapcontext(&(t_main_fiber->m_ctx),&m_ctx)){
            SYLAR_ASSERT2(false,"swapcontext");
      }
  }
  void Fiber::back(){
      SetThis(t_main_fiber.get());  
      if(swapcontext(&m_ctx,&t_main_fiber->m_ctx))
           SYLAR_ASSERT2(false,"swapcontext");
  }
  void Fiber::YieldToReady(){
      //获取current fiber 
    Fiber::ptr cur = GetThis();
    cur->m_state = READY;
    cur->swapOut();
  }
  void Fiber::YieldToHold(){
    Fiber::ptr cur = GetThis();
    cur->m_state = HOLD;
    cur->swapOut();
  }

  uint64_t Fiber::TotalFibers(){
        return s_fiber_count;
  }

  uint64_t Fiber::GetFiberId(){
        if(t_fiber)
          return t_fiber->getId();
        return 0;
  }
  void Fiber::MainFunc(){
     SYLAR_LOG_INFO(g_logger) << "MainFunc";
    //获取到current fiber,执行其函数
      Fiber::ptr cur = GetThis();
      SYLAR_ASSERT((cur));
      try{
          SYLAR_LOG_INFO(g_logger) << "Fiber Start Task";
          cur->m_cb();
          SYLAR_LOG_INFO(g_logger) << "Fiber finished Task";
          cur->m_cb  = nullptr;
          cur->m_state = TERM;
      }catch(std::exception& e){
          cur->m_state = EXCPT;
          SYLAR_LOG_ERROR(g_logger) << "Fiber Execpt:" << e.what() << std::endl << sylar::BackTraceToString(10);
      }catch(...){
          cur->m_state = EXCPT;
          SYLAR_LOG_ERROR(g_logger) << "Fiber Execpt:"<< std::endl << sylar::BackTraceToString(10);
      }
      //要用raw ptr切换回main fiber 
      //若用shared_ptr切换回main fiber ---> MainFunc没有结束 ---> cur在栈上，含有引用计数 ---> 不会释放fiber对象
      auto raw_ptr = cur.get();
      cur.reset();
      raw_ptr->swapOut();
      SYLAR_ASSERT2(false,"never swap back fiber id="+std::to_string(raw_ptr->getId()));
  }

  void Fiber::CallerMainFunc(){
     SYLAR_LOG_INFO(g_logger) << "CallerMainFunc";
      Fiber::ptr cur = GetThis();
      SYLAR_ASSERT((cur));
      try{
          SYLAR_LOG_INFO(g_logger) << "Fiber Start Task";
          cur->m_cb();
          SYLAR_LOG_INFO(g_logger) << "Fiber finished Task";
          cur->m_state = TERM;
          cur->m_cb  = nullptr;
      }catch(std::exception& e){
          cur->m_state = EXCPT;
          SYLAR_LOG_ERROR(g_logger) << "Fiber Execpt:" << e.what() << std::endl << sylar::BackTraceToString(10);
      }catch(...){
          cur->m_state = EXCPT;
          SYLAR_LOG_ERROR(g_logger) << "Fiber Execpt:"<< std::endl << sylar::BackTraceToString(10);
      }
      auto raw_ptr = cur.get();
      cur.reset();
      raw_ptr->back();
      SYLAR_ASSERT2(false,"never swap back fiber id="+std::to_string(raw_ptr->getId()));

  }
}
