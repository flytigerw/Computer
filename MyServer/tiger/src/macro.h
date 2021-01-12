
#ifndef MY_MACRO_H
#define MY_MACRO_H 

#include <assert.h>
#include "log.h"
#include "util.h"

  //assert 封装
#define MY_ASSERT(expr) \
  if(!(expr)){ \
    TIGER_LOG_ERROR(TIGER_LOG_ROOT) << "assert: " #expr "failed\n" \
                                      << "backtrace:\n" \
                                      << tiger::m_backtrace(); \
    assert((expr));\
  }


#define MY_ASSERT2(expr,info) \
  if(!expr){ \
    TIGER_LOG_ERROR(TIGER_LOG_ROOT) << "assert failed\n" \
                                             << "hint info:" << #info << "\n" \
                                             << "backtrace:\n" \
                                             << m_backtrace(); \
    assert(expr);\
  }

#endif
