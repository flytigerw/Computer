



#ifndef  CONSTUCT 
#define  CONSTUCT 

#include "iterator.h"
#include <new>
#include <type_traits>

namespace stl{
//在内存空间上对对象进行构造和析构
template<class T1,class T2>
inline 
void construct(T1* p,const T2& value){
  new(p)T1(value);
}

template<class T>
inline 
void desctruct(T* p){
  p->~T();
}
//手动调用析构函数
template<class ForwardIterator>
inline 
void destory_range_impl(ForwardIterator begin,ForwardIterator end,std::false_type){
  while(begin != end){
    destory(&*begin);
    ++begin;
  }
}

//若指向的元素含有 trivial-desctructor,则什么也不做让系统析构
template<class ForwardIterator>
inline 
void destory_range_impl(ForwardIterator,ForwardIterator ,std::true_type){}



//对范围内的元素进行析构
template<class ForwardIterator>
inline
void destory(ForwardIterator begin,ForwardIterator end){
  //考察指向的元素是否有trivial-desctructor,然后进行function dispatch 
  typedef typename std::is_trivially_destructible<
              typename iterator_traits<ForwardIterator>::value_type
              >::type t;
  destory_range_impl(begin,end,t());
}

//将[first11,last1)拷贝到[result,result+(last1-first1)
//将[first2,last2) 拷贝到[result+(last1-first1),~~)
template<class InputIterator1,class InputIterator2,class ForwardIterator>
inline ForwardIterator 
uninitialized_copy_copy(InputIterator1 first1,InputIterator1 last1,InputIterator2 first2,InputIterator2 last2, ForwardIterator result){
  auto mid = uninitialized_copy(first1,last1,result);
  try{
    return uninitialized_copy(first2,last2,mid);
  }catch(...){
    destory(result,mid);
    throw ;
  }
}

//fill和copy的再一个大区间里的混合使用
//将[result,mid)填充为x
//将[first,last)拷贝到[mid,mid+(last-first))
template<class ForwardIterator,class T,class InputIterator> 
inline ForwardIterator 
uninitialized_fill_copy(ForwardIterator result,ForwardIterator mid,const T& x,InputIterator first,InputIterator last){
  uninitialized_fill(result,mid,x);
  try{
      return uninitialized_copy(first,last,mid);
  }catch(...){
    destory(result,mid);
      throw ;
  }
}

//将[first,last11)拷贝到[first2,first2+(last1-first1))
//将[first2+(last1-first1),last2)填充为x
template<class ForwardIterator,class T,class InputIterator> 
inline ForwardIterator 
uninitialized_copy_fill(ForwardIterator first1,ForwardIterator last1,InputIterator first2,InputIterator last2,const T& x){
  auto mid = uninitialized_copy(first1,last1,first2);
  try{
      uninitialized_fill(mid,last2,x);
  }catch(...){
    destory(first2,mid);
      throw ;
  }
}

}
#endif
