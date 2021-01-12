
#ifndef UTIL_H
#define UTIL_H 

#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <sys/time.h>
#include <byteswap.h>
#include <cxxabi.h>


#define SYLAR_LITTLE_ENDIAN 1
#define SYLAR_BIG_ENDIAN 1
namespace sylar{
  pid_t GetThreadId();
  uint64_t GetFiberId();
  //获得整个函数栈 --> 通过vector存储每一函数frame
  //size为层数
  //skip ---> 越过前面几层
  void BackTrace(std::vector<std::string>& bt,int size,int skip = 1);
  //获得整个函数栈 --> 整体以string表示
  //先调用上面的函数，则通过strignstream转为string
  std::string BackTraceToString(int size,int skip = 2,const std::string& prefix = "");

  //时间相关
  uint64_t GetCurrentMS();
  uint64_t GetCurrentUS();

  template<class T>
const char* TypeToName() {
    static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    return s_name;
}



  //针对uint64_t的字节序转换
  template<class T>
    typename std::enable_if<sizeof(T)==sizeof(uint64_t),T>::type 
    byteswap(T value){
      return (T)bswap_64((uint64_t)value);
    }
  //32位
  template<class T>
    typename std::enable_if<sizeof(T)==sizeof(uint32_t),T>::type 
    byteswap(T value){
      return (T)bswap_32((uint32_t)value);
    }
  template<class T>
    typename std::enable_if<sizeof(T)==sizeof(uint16_t),T>::type 
    byteswap(T value){
      return (T)bswap_16((uint16_t)value);
    }

#if BYTE_ORDER  ==  BIG_ENDIAN 
#define SYLAR_BYTE_ORDER  BYTE_ORDER 
#else 
#define SYLAR_BYTE_ORDER SYLAR_LITTLE_ENDIAN
#endif 

#if SYLAR_BYTE_ORDER == SYLAR_BIG_ENDIAN 
  //一套函数:有些为默认情况，有些针对条件设立
  template<class T>
   T byteswapOnLittleEndian(T t){
      return byteswap(t);
    }

  template<class T>
    T byteswapOnBigEndian(T t){
      return t;
    }
#else
  template<class T>
   T byteswapOnLittleEndian(T t){
      return t;
    }

  template<class T>
    T byteswapOnBigEndian(T t){
     return byteswap(t);
    }

#endif


}



#endif
