
#include "context.h"
#include <unistd.h>
#include <sys/mman.h>


namespace tiger{
  

    //心存疑问，这个size用来干嘛?
    bool StackPart::ProtectStack(void* stack, size_t size, int pages)
    {
        if (!pages) 
          return false;

        if ((int)size <= getpagesize() * (pages + 1))
            return false;

        //将栈基址对齐到页
        void *protect_page_addr = ((size_t)stack & 0xfff) ? 
                                   (void*)(((size_t)stack & ~(size_t)0xfff) + 0x1000)
                                   : stack;

        if (-1 == mprotect(protect_page_addr, getpagesize() * pages, PROT_NONE)) 
          //日志打印
            return false;
        else 
            return true;
    }
    void StackPart::CancelProtect(void *stack, int pages)
    {
        if (!pages) 
          return ;

      
        void *protect_page_addr = ((size_t)stack & 0xfff) ? 
                                   (void*)(((size_t)stack & ~(size_t)0xfff) + 0x1000)
                                   : stack;

        if (-1 == mprotect(protect_page_addr, getpagesize() * pages, PROT_READ|PROT_WRITE)) {
          //日志打印
        } else {
        }

    }

}
