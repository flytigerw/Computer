#include "fiber.h"
#include <list>
#include <atomic>
#include <map>
#include "util.h"
#include "config.h"
#include "log.h"


namespace tiger{

static Logger::ptr g_logger = TIGER_LOG_LOGGER("system");
  
  static thread_local Fiber::ptr main_fiber = nullptr;
  static thread_local Fiber::ptr cur_fiber = nullptr;
  static thread_local std::map<uint64_t,Fiber::ptr> fiber_pool;

  static thread_local std::atomic<int> fiber_id{0};
  static thread_local std::atomic<int> fiber_count{0};

  //在堆上分配内存供栈使用
  class StackAllocator{
    public:
      static void* alloc(size_t size){
        return malloc(size);
      }
      static void delloc(void* p){
        free(p);
      }
  };
  //栈的配置项
  static ConfigItem<size_t>::ptr fiber_stack_szie_config = ConfigManager::LookUp<size_t>("fiber.stack.size",1024*1024,"fiber stack size");

  Fiber::Fiber(std::function<void()> cb,const std::string& name,size_t stacksize):m_cb(cb),m_name(name),m_stacksize(stacksize){
    //成员变量的构建
    if(m_name.empty())
      m_name = std::to_string(GetThreadId()) + "-" + std::to_string(++fiber_id);
    
    if(m_stacksize == 0)
      m_stacksize = fiber_stack_szie_config->get_value();
    m_id = fiber_id;
    m_stack = StackAllocator::alloc(m_stacksize);
    getcontext(&m_ctx);
    //后续的都采用swapcontext
    m_ctx.uc_link  = nullptr;
class Context
{
public:
  //传入参数:执行函数体，vp，栈大小
    Context(fn_t fn, intptr_t vp, std::size_t stackSize)
        : fn_(fn), vp_(vp), stackSize_(stackSize)
    {
      //分配栈
        stack_ = (char*)StackTraits::MallocFunc()(stackSize_);
        DebugPrint(dbg_task, "valloc stack. size=%u ptr=%p",
                stackSize_, stack_);
      //创建上下文
        ctx_ = make_fcontext(stack_ + stackSize_, stackSize_, fn_);
        //栈保护    
        int protectPage = StackTraits::GetProtectStackPageSize();
        if (protectPage && StackTraits::ProtectStack(stack_, stackSize_, protectPage))
            protectPage_ = protectPage;
    }

    ~Context()
    {
      //栈空间释放
        if (stack_) {
            DebugPrint(dbg_task, "free stack. ptr=%p", stack_);
            if (protectPage_)
                StackTraits::UnprotectStack(stack_, protectPage_);
            StackTraits::FreeFunc()(stack_);
            stack_ = NULL;
        }
    }

    //从当前上下文切换到此ctx中
    ALWAYS_INLINE void SwapIn()
    {
        jump_fcontext(&GetTlsContext(), ctx_, vp_);
    }

    //切换到指定ctx
    ALWAYS_INLINE void SwapTo(Context & other)
    {
        jump_fcontext(&ctx_, other.ctx_, other.vp_);
    }
    
    //切换到主协程
    ALWAYS_INLINE void SwapOut()
    {
        jump_fcontext(&ctx_, GetTlsContext(), 0);
    }

    //主协程
    fcontext_t& GetTlsContext()
    {
        static thread_local fcontext_t tls_context;
        return tls_context;
    }

private:
    //上下文环境
    fcontext_t ctx_;    //跳转的核心
    fn_t fn_;           //协程运行函数
    intptr_t vp_;       //当前上下文属于的 协程Task对象指针
    //栈，栈的保护页数
    char* stack_ = nullptr;
    uint32_t stackSize_ = 0;
    int protectPage_ = 0;
};
} // namespace co
    m_ctx.uc_stack.ss_sp  = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    //为协程添加任务
    makecontext(&m_ctx,&Fiber::run,0);
    ++fiber_count;
    fiber_pool.insert({m_id,shared_from_this()});
    //确保main_fiber在管理fiber_pool时，其里面已经含有fiber
    if(main_fiber == nullptr)
      main_fiber.reset(new Fiber);
  }

  //创建主协程
  Fiber::Fiber(){
    m_name = 
      m_name = std::to_string(GetThreadId()) + "-" + "main fiber";
    m_id = 0;
    if(m_stacksize == 0)
      m_stacksize = fiber_stack_szie_config->get_value();
    m_stack = StackAllocator::alloc(m_stacksize);
    getcontext(&m_ctx);
    m_ctx.uc_link  = nullptr;
    m_ctx.uc_stack.ss_sp  = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    makecontext(&m_ctx,&Fiber::main_run,0);
    ++fiber_count;
  }

  //析构:释放资源
  Fiber::~Fiber(){
    //先回收栈资源
    if(m_stack)
      StackAllocator::delloc(m_stack);
    --fiber_count;
  }
  

  //获取到当前协程，调用其任务
  void Fiber::run(){
    Fiber::ptr cur = GetCurFiber();
    try{
        TIGER_LOG_INFO(g_logger) << "Fiber starts Task";
        cur->m_cb();
        TIGER_LOG_INFO(g_logger) << "Fiber finished Task";
        //任务返回
        cur->m_cb  = nullptr;
        cur->m_state = TERM;
    }catch(std::exception& e){
        cur->m_state = EXCPT;
        TIGER_LOG_ERROR(g_logger) << "Fiber Execpt:" << e.what() << "\n" ;
        m_backtrace();
    }catch(...){
        cur->m_state = EXCPT;
        TIGER_LOG_ERROR(g_logger) << "Fiber Execpt:"  << "\n" ;
        m_backtrace();
    }
    
    fiber_pool.erase(cur->m_id);
    cur_fiber = main_fiber;
    setcontext(&main_fiber->m_ctx);
  }







  uint64_t Fiber::GetCurFiberId(){
    if(cur_fiber)
       return cur_fiber->m_id;
    return -1;
  }
  void Fiber::SetCurFiber(std::shared_ptr<Fiber> f){
    cur_fiber = f;
  }
  std::shared_ptr<Fiber> Fiber::GetCurFiber(){
    return cur_fiber;
  }
    



}
