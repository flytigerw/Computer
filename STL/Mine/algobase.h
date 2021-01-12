

#ifndef ALGOBASE
#define ALGOBASE 

#include "type_traits"
#include "iterator.h"
namespace stl{

//随机迭代器:以n决定循环次数，更快一些
template<class RandomAccessIterator,class OutputIterator,class Distance>
inline OutputIterator 
copy_assign(RandomAccessIterator first,RandomAccessIterator last,OutputIterator dest,Distance*){
  //元素之间间隔Distance 
  //对每个元素调用copy =
  for(Distance n = last-first; n>0;--n,++first,++dest )
    *dest = *first;
  return dest;
}


//若元素可以trivially assign,则直接利用memmove进行内存拷贝
//否则会使用元素的copy =
template<class T>
inline T* 
copy_impl(const T* first,const T* last,T* dest,std::true_type){
      memmove(dest,first,sizeof(T)*(last-first));
      return dest+(last-first);
}

template<class T>
inline T* 
copy_impl(const T* first,const T* last,T* dest,std::false_type){
    //同时也为随机迭代器
    return copy_assign(first,last,dest,(ptrdiff_t*)0);
}


template<class InputIterator,class OutputIterator>
inline OutputIterator 
copy_tag(InputIterator first,InputIterator last,OutputIterator dest,input_iterator_tag){
      for(;first != last; ++dest,++first)
        *dest = *first;
      return dest;
}
//随机迭代器 
template<class RandomAccessIterator,class OutputIterator>
inline OutputIterator 
copy_tag(RandomAccessIterator first,RandomAccessIterator last,OutputIterator dest,random_access_iterator_tag){
      return copy_assign(first,last,dest,distance_type(first));
}

template <class InputIterator, class OutputIterator>
struct copy_dispatch
{
  OutputIterator operator()(InputIterator first, InputIterator last,
                            OutputIterator result) {
    //根据迭代器tag再分发
    return copy_tag(first, last, result, iterator_category(first));
  }
};

//迭代器类型为:(const T*,T*)
template<class T>
struct copy_dispatch<const T*,T*>{
  T* operator()(const T* first,const T* last,T* dest){
      copy_impl(first,last,dest,typename std::is_trivially_copy_assignable<T>::type());
  }
};

//迭代器类型为:(T*,T*)
template<class T>
struct copy_dispatch<T*,T*>{
  T* operator()(const T* first,const T* last,T* dest){
    return copy_impl(first,last,dest,typename std::is_trivially_copy_assignable<T>::type());
  }
};

//copy  src:[first,last]   dest:[dest --- 
//返回dest+n
//要求目标区间默认比源区间大
template<class InputIterator,class OutputIterator>
inline OutputIterator
copy(InputIterator first,InputIterator last,OutputIterator dest){
    
    //根据迭代器类型进行分发
   return  copy_dispatch<InputIterator,OutputIterator>()(first,last,dest) ;  
}

//对范围[fisrt,last]重新填值 
template<class  ForwardIterator, class T>
void fill(ForwardIterator first,ForwardIterator last,const T& value){
    for(;first != last; first++)
      *first = value;
}

//返回最后被填入的下一个位置
//若操作区间超过容器范围,会导致不可预期的结果
//故常用具有插入而非复写能力的迭代器--> insert_iterator 
//fill_n(back_insert(vector),xx);
template<class  ForwardIterator, class Size,class T>
ForwardIterator fill_n(ForwardIterator first,Size n,const T& value){
  for(; n>0;--n,++first)
    *first = value;
    return first;
}



//从后向前拷贝
template <class BidirectionalIterator1, class BidirectionalIterator2>
inline BidirectionalIterator2 copy_backward_impl(BidirectionalIterator1 first, 
                                              BidirectionalIterator1 last, 
                                              BidirectionalIterator2 result) {
  while (first != last) *--result = *--last;
  return result;
}


template <class BidirectionalIterator1, class BidirectionalIterator2>
struct copy_backward_dispatch
{
  BidirectionalIterator2 operator()(BidirectionalIterator1 first, 
                                    BidirectionalIterator1 last, 
                                    BidirectionalIterator2 result) {
    return copy_backward_impl(first, last, result);
  }
};


template <class T>
inline T* copy_backward_dispatch__(const T* first, const T* last, T* result,std::true_type) {
  const ptrdiff_t N = last - first;
  memmove(result - N, first, sizeof(T) * N);
  return result - N;
}

template <class T>
inline T* copy_backward_dispatch__(const T* first, const T* last, T* result,std::false_type) {
  return copy_backward_impl(first, last, result);
}

template <class T>
struct copy_backward_dispatch<T*, T*>
{
  T* operator()(T* first, T* last, T* result) {
    return copy_backward_dispatch__(first, last, result,typename std::is_trivially_copy_assignable<T>::type());
  }
};

template <class T>
struct copy_backward_dispatch<const T*, T*>
{
  T* operator()(const T* first, const T* last, T* result) {
    //根据所知对象是否可以trivially assign进行分发
    return copy_backward_dispatch__(first, last, result,typename std::is_trivially_copy_assignable<T>::type());
  }
};


//根据迭代器类型进行分发
template <class BidirectionalIterator1, class BidirectionalIterator2>
inline BidirectionalIterator2 copy_backward(BidirectionalIterator1 first, 
                                            BidirectionalIterator1 last, 
                                            BidirectionalIterator2 result) {
  return copy_backward_dispatch<BidirectionalIterator1,BidirectionalIterator2>()(first,last,result);
}


//逐个元素比较 --> 查看范围B里的元素是否逐个严格<范围A的元素
template <class InputIterator1, class InputIterator2>
bool lexicographical_compare(InputIterator1 first1, InputIterator1 last1,
			     InputIterator2 first2, InputIterator2 last2) {
  for ( ; first1 != last1 && first2 != last2; ++first1, ++first2) {
    if (*first1 < *first2)
      return true;
    if (*first2 < *first1)
      return false;
  }
  return first1 == last1 && first2 != last2;
}
template <class InputIterator1, class InputIterator2, class Compare>
bool lexicographical_compare(InputIterator1 first1, InputIterator1 last1,
			     InputIterator2 first2, InputIterator2 last2,
			     Compare comp) {
  for ( ; first1 != last1 && first2 != last2; ++first1, ++first2) {
    if (comp(*first1, *first2))
      return true;
    if (comp(*first2, *first1))
      return false;
  }
  return first1 == last1 && first2 != last2;
}
}
#endif
