#ifndef MY_UTIL_H
#define MY_UTIL_H 

#include <execinfo.h>
#include <memory>
#include <iostream>

#include "thread.h"

namespace tiger{

  std::string m_backtrace(int beg=2,int size=100);
  uint32_t GetThreadId();
  uint32_t GetFiberId();
  std::string GetThreadName();


}


#endif
