#ifndef TEMPORARY_BUFFER
#define TEMPORARY_BUFFER 

#include "iterator.h"
#include "uninitialized.h"
#include <type_traits>
#include "construct.h"

namespace stl{
  //临时的缓冲区
  template<class T>
    std::pair<T*,ptrdiff_t> get_temporary_buffer(ptrdiff_t len,T*){
    if (len > ptrdiff_t( __INT_MAX__ / sizeof(T)))
      len = __INT_MAX__ / sizeof(T);
 
    while (len > 0) {
      T* tmp = (T*) malloc((size_t)len * sizeof(T));
      if (tmp != 0)
        return std::pair<T*, ptrdiff_t>(tmp, len);
      len /= 2;
    }
  }
  template <class T>
   void return_temporary_buffer(T* p) {
     free(p);
   }

template<class ForwardIterator,class T=typename iterator_traits<ForwardIterator>::value_type>
  class temporary_buffer{
    private:
      ptrdiff_t original_len;   //请求长度
      ptrdiff_t len;            //实际长度
      T* buffer;
      
      void allocate_buffer(){
        original_len = len;
        buffer = nullptr;
        //限制缓冲区长度
        if(len > (ptrdiff_t)(__INT_MAX__)/sizeof(T))
          len = __INT_MAX__ / sizeof(T);
        while(len>0){
          buffer = (T*)malloc(len*sizeof(T));
          //分配成功就就退出，若没有，则减小尺寸，继续申请
          if(buffer)
            break;
          len /= 2;
        }
      }
      //可以默认构造，什么也不做
      void initialize_buffer(const T&,std::true_type){}
      void initialize_buffer(const T& v,std::false_type){
        //填充len个元素v
        uninitialized_fill_n(buffer,len,v) ;      
      }
    public:
      ptrdiff_t size() const {return len;}
      ptrdiff_t requested_size() const{return original_len;}
      T* begin() {return buffer;}
      T* end(){return buffer+len;}
      //用一对前向迭代器表示的范围进行初始化
      temporary_buffer(ForwardIterator first,ForwardIterator last){
        try{
          len = 0;
          distance(first,last,len);
          allocate_buffer();
          if(len>0)
            initialize_buffer(*first,std::is_trivially_default_constructable<T>::type);
        }catch(...){
          free(buffer);
          buffer = nullptr;
          len = 0;
          throw ;
        }
      }
      ~temporary_buffer(){
        destory(buffer,buffer+len);
        free(buffer);
      }
    temporary_buffer(const temporary_buffer&) = delete;
    temporary_buffer& operator=(const temporary_buffer&) = delete;
  };
}

#endif
