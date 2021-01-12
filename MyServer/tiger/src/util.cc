#include "util.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sstream>
#include <string>


namespace tiger{
  //将各个函数栈的信息输出
  //参数:函数栈的层次范围
  std::string m_backtrace(int beg,int size) {
    std::stringstream ss;
    void** buf = (void**) malloc(sizeof(void*)*size);
    int s = backtrace(buf,size);
    //将其符号化
    char** strings = backtrace_symbols(buf,s);

    if(strings == nullptr){
        std::cout << "backtrace_symbols fail" << std::endl;
        free(buf);
        free(strings);
        return ss.str();
    }

    for(int i=beg;i<s;i++)
      ss << strings[i] << std::endl;
     free(buf);
     free(strings);
     return ss.str();
  }

  uint32_t GetThreadId(){
    return syscall(SYS_gettid);
  }

  uint32_t GetFiberId(){
    return 0;
  }

  
  std::string GetThreadName(){
    return "unknown name";
  }
  
}
