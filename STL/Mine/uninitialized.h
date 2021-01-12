
#ifndef UNINITIALIZED
#define UNINITIALIZED 

#include "iterator.h"
#include "alloc.h"
#include "algobase.h"
  
namespace stl{
  //POD类型
  template<class InputIterator,class OutputIterator>
    inline OutputIterator 
    uninitialized_copy_impl(InputIterator first,InputIterator last,OutputIterator dest,std::true_type){
      //直接进行内存拷贝
      return copy(first,last,dest);
    }
  //非POD类型采用construct进行构造 --> 会调用对象的构造函数
  template<class InputIterator,class OutputIterator>
    inline OutputIterator 
    uninitialized_copy_impl(InputIterator first,InputIterator last,OutputIterator dest,std::false_type){
      auto cur = dest;
      try{
        for(; first != last; ++first,++cur)
          construct(&*cur,*first);
      }catch(...){
        destory(dest,cur);
        throw ;
      }
    }
  
  template<class InputIterator,class OutputIterator,class T>
    inline OutputIterator 
    uninitialized_copy_(InputIterator first,InputIterator last,OutputIterator desc,T*){
      //根据迭代器所指对象是否为POD类型进行分发
      return uninitialized_copy_impl(first,last,desc,typename std::is_pod<T>::type());
    }
  //拷贝构造
  template<class InputIterator,class OutputIterator>
    inline OutputIterator 
    uninitialized_copy(InputIterator first,InputIterator last,OutputIterator dest){
      return uninitialized_copy_(first,last,dest,value_type(first));
    }
    
  //针对char和wchar_t的片特化版本 ----> 使用memmove
  inline char* uninitialized_copy(const char* first,const char* last,char* dest){
    memmove(dest,first,(last-first));
    return dest+(last-first);
  }
  inline wchar_t* uninitialized_copy(const wchar_t* first,const wchar_t* last,wchar_t* dest){
    memmove(dest,first,sizeof(wchar_t)*(last-first));
    return dest+(last-first);
  }
  
  template<class ForwardIterator,class T>
    inline void 
    uninitialized_fill_impl(ForwardIterator first,ForwardIterator last,const T&x,std::true_type){
      fill(first,last,x);    
    }
  
   template<class ForwardIterator,class T>
    inline void 
    uninitialized_fill_impl(ForwardIterator first,ForwardIterator last,const T&x,std::false_type){
      auto cur = first;
      //commit -- rollback
      try{
          for(;cur != last; ++cur)
            construt(&*cur,x);
      }catch(...){
        destory(first,cur);
        throw ;
      }
    }
  template<class ForwardIterator,class T1,class T>
    inline void
    uninitialized_fill_(ForwardIterator first,ForwardIterator last,const T1& x,T*){
      return uninitialized_fill_impl(first,last,x,typename std::is_pod<T>::type());
    }

   //将[first,last]都填充为x
  template<class ForwardIterator,class T>
    inline void 
    uninitialized_fill(ForwardIterator first,ForwardIterator last,const T& x){
      uninitialized_fill_(first,last,x,value_type(first));
    }
   //填充范围[first,first+n]
template <class ForwardIterator, class Size, class T>
inline ForwardIterator
uninitialized_fill_n_impl(ForwardIterator first, Size n,
                           const T& x, std::true_type) {
  return fill_n(first, n, x);
}

template <class ForwardIterator, class Size, class T>
ForwardIterator
uninitialized_fill_n_impl(ForwardIterator first, Size n,
                           const T& x, std::false_type) {
  ForwardIterator cur = first;
  try{
    for ( ; n > 0; --n, ++cur)
      construct(&*cur, x);
    return cur;
  }catch(...){
    destroy(first, cur);
    throw ;
  }
}

template <class ForwardIterator, class Size, class T, class T1>
inline ForwardIterator uninitialized_fill_n_(ForwardIterator first, Size n,
                                              const T& x, T1*) {
  return uninitialized_fill_n_impl(first, n, x, typename std::is_pod<T1>::type());
}

template <class ForwardIterator, class Size, class T>
inline ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n,
                                            const T& x) {
  return uninitialized_fill_n_(first, n, x, value_type(first));
}

}
#endif
