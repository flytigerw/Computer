#pragma once 
#include "fcontext.h"
#include <malloc.h>


namespace tiger{

  
  //栈相关
  //栈分配，栈保护
  
  struct  StackPart{
    //静态变量，采用函数获取
    //要保护的栈的大小
    static int& GetProtectStackPageSize(){
      static int size = 0;
      return size;
    }

    //参数:栈基址，要保护的大小，页面数量
    static bool ProtectStack(void* stack,size_t size,int pages);
    static void CancelProtect(void* stack,int pages);

    static void* alloc(size_t size){
      return malloc(size);
    }
    static void delloc(void* p){
      free(p);
    }

  };

class Context
{
public:
    //参数:执行函数体，vp，栈大小
    Context(fn_t fn, intptr_t Task,size_t stacksize): m_task(fn), m_fiber(Task), m_stacksize(stacksize)
    {
        //分配栈
        m_stack = (char*)StackPart::alloc(m_stacksize);
        //创建上下文
        //参数:栈，执行函数
        m_ctx = make_fcontext(m_stack + m_stacksize, m_stacksize, m_task);

        //栈保护
        int protect_page = StackPart::GetProtectStackPageSize();   //由配置系统配置要保护的栈大小
        if (protect_page && StackPart::ProtectStack(m_stack, m_stacksize, protect_page))
           m_protect_page = protect_page;
    }

    //释放栈空间，以及取消栈保护
    ~Context()
    {
        if (m_stack) {
        //    DebugPrint(dbg_task, "free stack. ptr=%p", m_stack);
            if (m_protect_page)
                StackPart::CancelProtect(m_stack, m_protect_page);
            StackPart::delloc(m_stack);
            m_stack = NULL;
        }
    }

    //从当前上下文切换到此ctx中
    void swap_in()
    {
        jump_fcontext(&get_main_context(), m_ctx, m_fiber);
    }

    //切换到指定ctx
    void swap_to(Context & other)
    {
        jump_fcontext(&m_ctx, other.m_ctx,other.m_fiber);
    }
    
    //切换到主协程
    void swap_out()
    {
        jump_fcontext(&m_ctx, get_main_context(), 0);
    }

    //每个线程都有一个main_context  ---> thread_local性质
    fcontext_t& get_main_context()
    {
        static thread_local fcontext_t main_context;
        return main_context;
    }

private:
    //成员:上下文环境,要执行的任务,该上下文属于哪一个fiber对象，栈相关

    fcontext_t m_ctx;    
    fn_t m_task;           
    intptr_t m_fiber;       
    //栈，栈的保护页数
    char* m_stack = nullptr;
    uint32_t m_stacksize = 0;
    int m_protect_page = 0;
};

} 


