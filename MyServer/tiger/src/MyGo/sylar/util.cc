#include "util.h"
#include <execinfo.h>
#include "log.h"
#include <sstream>
#include <sys/stat.h>
#include "fiber.h"

namespace sylar{
  static Logger::ptr g_logger = SYLAR_LOG_LOGGER("system");
  pid_t GetThreadId(){
    return syscall(SYS_gettid);
  }
  uint64_t GetFiberId(){
    return Fiber::GetFiberId();
  }
  void BackTrace(std::vector<std::string>& bt,int size,int skip){
    //array中的每个元素为void*
    void** array = (void**)malloc((sizeof(void*)*size));
    //s为实际获得的层数
    size_t s = ::backtrace(array,size);
    //将其符号化 ---> 增加可读性
    char** strings = backtrace_symbols(array,s);
    if(strings == NULL){
      SYLAR_LOG_ERROR(g_logger) << "backtrace_symbols error";
      free(strings);
      free(array);
      return;
    }
    //获取每一层
    for(size_t i=skip;i<s;i++)
      bt.push_back(strings[i]);
    free(strings);
    free(array);
  }
  std::string BackTraceToString(int size,int skip,const std::string& prefix){

    std::vector<std::string> bt;
    BackTrace(bt,size,skip);
    std::stringstream ss;
    for(size_t i=0;i<bt.size();i++)
      ss << prefix<< bt[i] << std::endl;
    return ss.str();
  }
  uint64_t GetCurrentMS(){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000 + tv.tv_usec / 1000;
  }
  uint64_t GetCurrentUS(){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000*1000 + tv.tv_usec;
  }


}
