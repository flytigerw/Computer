
#ifndef __MARCRO_H
#define __MARCRO_H

#include <string.h>

#include <assert.h>
#include "util.h"

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立            ----> 用于if语句中，加速编译
#   define SYLAR_LIKELY(x)       __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#   define SYLAR_UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
#   define SYLAR_LIKELY(x)      (x)
#   define SYLAR_UNLIKELY(x)      (x)
#endif

//assert(logic expr) ---> 为false时会abort
#define SYLAR_ASSERT(x) \
  if(!(x)){ \
    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION:" << x \
                                      << "\nbacktrace:\n" \
                                      << sylar::BackTraceToString(100,2,"   "); \
    assert(x); \
  }
//含有额外信息w
#define SYLAR_ASSERT2(x,w) \
  if(!(x)){ \
    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION:" << x \
                                      <<  "\n" << w \
                                      << "\nbacktrace:\n" \
                                      << sylar::BackTraceToString(100,2,"   "); \
    assert(x); \
  }
#endif
