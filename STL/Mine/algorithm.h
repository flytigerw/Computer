#ifndef ALGORITHM
#define ALGORITHM 

#include "iterator.h"
#include <type_traits>
#include <string.h>
#include "temporary_buffer.h"
#include "heap_algorithm.h"
#include "algobase.h"

namespace stl{
//范围1:[x,y],范围2[m,~]
//前提要求范围2>范围1
//判断范围1里的元素是否都包含在范围2
template<class InputIterator1,class InputIterator2>
inline bool 
is_subset(InputIterator1 first1,InputIterator1 last1,InputIterator2 first2){
  for(;first1 != last1;++first1,++first2)
    if(*first1 != *first2)
      return false;
  return true;
}
//自定义二元比较器
template<class InputIterator1,class InputIterator2,class BinaryPrediacte>
inline bool 
is_subset(InputIterator1 first1,InputIterator1 last1,InputIterator2 first2,BinaryPrediacte pred){
  for(;first1 != last1;++first1,++first2)
    if(!pred(*first1,*first2))
      return false;
  return true;
}




//ForwardIterator ---> 只能++ ---> 调用advance()进行多次++ 
template<class ForwardIterator,class T,class Distance>
ForwardIterator lower_bound_impl(ForwardIterator first,ForwardIterator last,const T& value,Distance*,stl::forward_iterator_tag){

    Distance len;
    distance(first,last,len);
    Distance half;
    ForwardIterator middle_iter;
    while(len > 0){
      half = len >> 1;
      middle_iter = first;
      advance(middle_iter,half);
      if(*middle_iter < value){
        first = middle_iter;
        ++first;
        len = len - half -1;
      }else 
        len = half;
    }
      return first;
}
//随机迭代器 ---> 可以+n
template<class RandomAccessIterator,class T,class Distance>
RandomAccessIterator lower_bound_impl(RandomAccessIterator first,RandomAccessIterator last,const T& value,Distance*,stl::random_access_iterator_tag){
    Distance len = last - first;
    Distance half;
    RandomAccessIterator middle_iter;
    while(len > 0){
      half = len >> 1;
      middle_iter = first+half;
      if(*middle_iter < value){ 
        //若下一个元素>= value则first就不会变动
        //缩小到后半区间
        first = middle_iter + 1;
        len = len-half - 1;
      }else 
        len = half;
    }
    return first;
}

///通过二分查找,在排序序列中找到第一个>=value的元素
template<class ForwardIterator,class T>
inline ForwardIterator 
lower_bound(ForwardIterator first,ForwardIterator last,const T& value){
  //根据迭代器类型进行分发
  return lower_bound_impl(first,last,value,distance_type(first),iterator_category(first));
}


//lower_bound自定义比较器版本
template<class ForwardIterator,class T,class Compare,class Distance>
ForwardIterator lower_bound_impl(ForwardIterator first,ForwardIterator last,const T& value,Compare comp,Distance*,stl::forward_iterator_tag){

    Distance len;
    distance(first,last,len);
    Distance half;
    ForwardIterator middle_iter;
    while(len > 0){
      half = len >> 1;
      middle_iter = first;
      advance(middle_iter,half);
      if(comp(*middle_iter, value)){
        first = middle_iter;
        ++first;
        len = len - half -1;
      }else 
        len = half;
    }
      return first;
}
//随机迭代器 ---> 可以+n
template<class RandomAccessIterator,class T,class Compare,class Distance>
RandomAccessIterator lower_bound_impl(RandomAccessIterator first,RandomAccessIterator last,const T& value,Compare comp,Distance*,stl::random_access_iterator_tag){
    Distance len = last - first;
    Distance half;
    RandomAccessIterator middle_iter;
    while(len > 0){
      half = len >> 1;
      middle_iter = first+half;
      if(comp(*middle_iter,value)){ 
        //缩小到后半区间
        first = middle_iter + 1;
        len = len-half - 1;
      }else 
        len = half;
    }
    return first;
}

template<class ForwardIterator,class T,class Compare>
inline ForwardIterator 
lower_bound(ForwardIterator first,ForwardIterator last,const T& value,Compare comp){
  //根据迭代器类型进行分发
  return lower_bound_impl(first,last,value,comp,distance_type(first),iterator_category(first));
}


//ForwardIterator
template<class ForwardIterator,class T,class Compare,class Distance>
ForwardIterator upper_bound_impl(ForwardIterator first,ForwardIterator last,const T& value,Compare comp,Distance*,stl::forward_iterator_tag){

    Distance len;
    distance(first,last,len);
    Distance half;
    ForwardIterator middle_iter;
    while(len > 0){
      half = len >> 1;
      middle_iter = first;
      advance(middle_iter,half);
      if(comp(*middle_iter,value)){
        first = middle_iter;
        ++first;
        len = len - half -1;
      }else 
        len = half;
    }
      return first;
}

template<class ForwardIterator,class T,class Distance>
ForwardIterator upper_bound_impl(ForwardIterator first,ForwardIterator last,const T& value,Distance*,stl::forward_iterator_tag){
    Distance len;
    distance(first,last,len);
    Distance half;
    ForwardIterator middle_iter;
    while(len > 0){
      half = len >> 1;
      middle_iter = first;
      advance(middle_iter,half);
      if(*middle_iter<=value){
        first = middle_iter;
        ++first;
        len = len - half -1;
      }else 
        len = half;
    }
      return first;
}

//随机迭代器
template<class RandomAccessIter,class T,class Distance,class Compare>
RandomAccessIter upper_bound_impl(RandomAccessIter first,RandomAccessIter last,const T& value,Distance*,Compare comp,stl::random_access_iterator_tag){
      Distance len = last - first;
      Distance half;
      RandomAccessIter middle_iter;
      while(len > 0){
        half = len >> 2;
        middle_iter = first + half;
        if(comp(*middle_iter,value)){
          first = middle_iter + 1;
          len = len-half-1;
        }else 
          len = half;
      }
      return first;
}


template<class RandomAccessIter,class T,class Distance>
RandomAccessIter upper_bound_impl(RandomAccessIter first,RandomAccessIter last,const T& value,Distance*,stl::random_access_iterator_tag){
      Distance len = last - first;
      Distance half;
      RandomAccessIter middle_iter;
      while(len > 0){
        half = len >> 2;
        middle_iter = first + half;
        if(*middle_iter <= value){
      //只用当value middle_iter值<=value时才变动 ---> 变动到下一个 
      //若下一个依旧<=value，则依旧会变动
      //若下一个严格>value,则不会变动
          first = middle_iter + 1;
          len = len-half-1;
        }else 
          len = half;
      }
      return first;
}


//根据二分查找,找到第一个>value的元素
template<class ForwardIterator,class T,class Compare>
inline ForwardIterator 
upper_bound(ForwardIterator first,ForwardIterator last,const T& value,Compare comp){
  //根据迭代器类型进行分发
  return upper_bound_impl(first,last,value,comp,distance_type(first),iterator_category(first));
}

template<class ForwardIterator,class T>
inline ForwardIterator 
upper_bound(ForwardIterator first,ForwardIterator last,const T& value){
  //根据迭代器类型进行分发
  return upper_bound_impl(first,last,value,distance_type(first),iterator_category(first));
}


//针对前向迭代器 ---> 只能++
template<class ForwardIterator,class T,class Distance>
std::pair<ForwardIterator,ForwardIterator>
equal_range__(ForwardIterator first,ForwardIterator last,const T& value,Distance*,forward_iterator_tag){
      Distance len =last-first;
      Distance half;
      ForwardIterator mid,left,right;
      //采用二分法
      while(len > 0){
        half = len>>1;
        mid = first;
        advance(mid,half);
        //过小，进入右边
        if(*mid < value){
          first = mid;
          ++first;
          len = len - half - 1;
          //过大，进入左边
        }else if(value <*mid)
          //只需要长度值即可 ---> 右边的临界重点不重要
          len = half;
        else{ 
          left = lower_bound(first,mid,value);
          right = upper_bound(++mid,first+len,value);
          return std::make_pair(left,right);
        }
      }
      //没找到
          return std::make_pair(first,first);
}

//针对随机迭代器 ---> 可以+n
template<class RandomAccessIter,class T,class Distance>
std::pair<RandomAccessIter,RandomAccessIter>
equal_range__(RandomAccessIter first,RandomAccessIter last,const T& value,Distance*,random_access_iterator_tag){
      Distance len =last-first;
      Distance half;
      RandomAccessIter mid,left,right;
      //采用二分法
      while(len > 0){
        half = len>>1;
        mid = first+half;
        //过小，进入右边
        if(*mid < value){
          first = mid+1;
          len = len - half - 1;
          //过大，进入左边
        }else if(value <*mid)
          //只需要长度值即可 ---> 右边的临界重点不重要
          len = half;
        else{  //序列可能为   xxxaaaaaaaayyyy
               //mid指向中间某一个a
               //那么就在[first,mid)通过lower_bound找到第一个a所在地
               //在[mid+1,last)中通过upper_bound找到第一个>a的所在地
          left = lower_bound(first,mid,value);
          right = upper_bound(++mid,first+len,value);
          return std::make_pair(left,right);
        }
      }
      //没找到
          return std::make_pair(first,first);
}

//在有序区间中找到=value的子区间
template<class ForwardIterator,class T>
inline std::pair<ForwardIterator,ForwardIterator>
equal_range(ForwardIterator first,ForwardIterator last, const T& value){
      //根据迭代器的种类进行分发
      return equal_range__(first,last,value,distance_type(first),iterator_category(first));
}


//自带比较器
template<class ForwardIterator,class T,class Distance,class Compare>
std::pair<ForwardIterator,ForwardIterator>
equal_range__(ForwardIterator first,ForwardIterator last,const T& value,Distance*,forward_iterator_tag,Compare comp){
      Distance len =last-first;
      Distance half;
      ForwardIterator mid,left,right;
      //采用二分法
      while(len > 0){
        half = len>>1;
        mid = first;
        advance(mid,half);
        //过小，进入右边
        if(comp(*mid,value)){
          first = mid;
          ++first;
          len = len - half - 1;
          //过大，进入左边
        }else if(comp(value,*mid))
          //只需要长度值即可 ---> 右边的临界重点不重要
          len = half;
        else{ 
          left = lower_bound(first,mid,value,comp);
          right = upper_bound(++mid,first+len,value,comp);
          return std::make_pair(left,right);
        }
      }
      //没找到
          return std::make_pair(first,first);
}

template<class RandomAccessIter,class T,class Distance,class Compare >
std::pair<RandomAccessIter,RandomAccessIter>
equal_range__(RandomAccessIter first,RandomAccessIter last,const T& value,Distance*,random_access_iterator_tag,Compare comp){
      Distance len =last-first;
      Distance half;
      RandomAccessIter mid,left,right;
      //采用二分法
      while(len > 0){
        half = len>>1;
        mid = first+half;
        //过小，进入右边
        if(comp(*mid,value)){
          first = mid+1;
          len = len - half - 1;
          //过大，进入左边
        }else if(comp(value,*mid))
          len = half;
        else{  
          left = lower_bound(first,mid,value,comp);
          right = upper_bound(++mid,first+len,value,comp);
          return std::make_pair(left,right);
        }
      }
          return std::make_pair(first,first);
}

//在有序区间中找到=value的子区间
template<class ForwardIterator,class T,class Compare>
inline std::pair<ForwardIterator,ForwardIterator>
equal_range(ForwardIterator first,ForwardIterator last, const T& value,Compare comp){
      //根据迭代器的种类进行分发
      return equal_range__(first,last,value,distance_type(first),iterator_category(first),comp);
}


//在有序区间中二分查找 --> 直接调用lower_bound
template <class ForwardIterator, class T>
bool binary_search(ForwardIterator first, ForwardIterator last,
                   const T& value) {
  ForwardIterator i = lower_bound(first, last, value);
  //可能没找到，进行检验 value>=*i
  return i != last && !(value < *i);
}

template <class ForwardIterator, class T, class Compare>
bool binary_search(ForwardIterator first, ForwardIterator last, const T& value,
                   Compare comp) {
  ForwardIterator i = lower_bound(first, last, value, comp);
  return i != last && !comp(value, *i);
}




//对范围内的每个元素施加func操作
template<class InputIterator,class Function>
Function for_each(InputIterator first,InputIterator last,Function f){
  for(;first!=last;++first)
    f(*first);
  return f;
}

//顺序查找元素x是否在这一区间中
//返回指向该元素的迭代器 ---> 没找到，返回last
template<class InputIterator,class T>
InputIterator find(InputIterator first,InputIterator last,const T& value){
  for(;first != last && *first != value;++first);
  return first;
}
//自带比较器版本
template<class InputIterator,class T,class Predicate>
InputIterator find_if(InputIterator first,InputIterator last,const T& value,Predicate pred){
  for(;first != last && !pred(*first ,value);++first);
  return first;
}

//找到第一对相邻的满足条件的元素的位置 --> 比如第一对相邻而又重复的
template<class ForwardIterator>
ForwardIterator adjacent_find(ForwardIterator first,ForwardIterator last){
    if(first == last)
      return last;
    //遍历,first表示上一个值
    auto cur = first;
    while(++cur != last){
      if(*first == *cur)
        return first;
      first = cur;
    }
    return last;
}

template<class ForwardIterator,class BinaryPrediacte>
ForwardIterator adjacent_find(ForwardIterator first,ForwardIterator last,BinaryPrediacte binary_pred){
    if(first == last)
      return last;
    //遍历,first表示上一个值
    auto cur = first;
    while(++cur != last){
      if(binary_pred(*first,*cur)) //二者满足某条件即可
        return first;
      first = cur;
    }
    return last;
}

//顺序查找x在区间里出现了多少次
template<class InputIterator,class T,class Size> 
void count(InputIterator first,InputIterator last,const T& x,Size& n){
  for(;first != last;++first)
    if(*first == x)
      ++n;
}

//区间内满足某条件的元素有多少个
template<class InputIterator,class Size,class Predicate> 
void count_if(InputIterator first,InputIterator last,Predicate pred,Size& n){
  for(;first != last;++first)
    if(pred(*first))
      ++n;
}
//通过返回值传递结果版本 ----> 需要知道返回类型 
template<class InputIterator,class T>
typename iterator_traits<InputIterator>::difference_type 
count(InputIterator first,InputIterator last,const T& x){
  
  typename iterator_traits<InputIterator>::difference_type n =0;
  for(;first != last;++first)
    if(*first == x)
      ++n;
  return n;
}
template<class InputIterator,class T,class Predicate>
typename iterator_traits<InputIterator>::difference_type 
count_if(InputIterator first,InputIterator last,Predicate pred){
  
  typename iterator_traits<InputIterator>::difference_type n =0;
  for(;first != last;++first)
    if(pred(*first))
      ++n;
  return n;
}



template<class ForwardIterator1,class ForwardIterator2,class Distance1,class Distance2>
inline ForwardIterator1 search(ForwardIterator1 first1,ForwardIterator1 last1,ForwardIterator2 first2,ForwardIterator2 last2,Distance1*,Distance2*){
  Distance1  d1 = 0;
  distance(first1,last1,d1);
  Distance1  d2 = 0;
  distance(first2,last2,d2);

  ForwardIterator1 cur1 = first1;
  ForwardIterator2 cur2 = first2;

  while(cur2 != last2){
    if(*cur1 == *cur2){  //匹配，两个窗口递增
        ++cur1;
        ++cur2;
    }else{
      if(d1 == d2){  //剩下的元素个数持平,不管找没找到，结果都是last1
        return last1;
      }else{
        //回溯，缩小范围
        cur1 = first1;
        cur2 = first2;
        --d1;
      }
    }
  }
}
//序列A,B找到序列B第一次在序列A中出现的位置
//采用滑动窗口+回溯的方法
template<class ForwardIterator1,class ForwardIterator2>
inline ForwardIterator1 search(ForwardIterator1 first1,ForwardIterator1 last1,ForwardIterator2 first2,ForwardIterator2 last2){
  //计算迭代器之间的距离时需要知道距离类型
  search_impl(first1,last1,first2,last2,distance_type(first1),distance_type(first2));
}


template<class ForwardIterator1,class ForwardIterator2,class Distance1,class Distance2,class Predicate>
inline ForwardIterator1 search(ForwardIterator1 first1,ForwardIterator1 last1,ForwardIterator2 first2,ForwardIterator2 last2,Distance1*,Distance2*,Predicate pred){
  Distance1  d1 = 0;
  distance(first1,last1,d1);
  Distance1  d2 = 0;
  distance(first2,last2,d2);

  ForwardIterator1 cur1 = first1;
  ForwardIterator2 cur2 = first2;

  while(cur2 != last2){
    if(pred(*cur1,*cur2)){  //匹配，两个窗口递增
        ++cur1;
        ++cur2;
    }else{
      if(d1 == d2){  //剩下的元素个数持平,不管找没找到，结果都是last1
        return last1;
      }else{
        //回溯，缩小范围
        cur1 = first1;
        cur2 = first2;
        --d1;
      }
    }
  }
}
//序列A,B找到序列B第一次在序列A中"匹配"的位置 --> 自定义匹配规则
template<class ForwardIterator1,class ForwardIterator2,class Predicate>
inline ForwardIterator1 search(ForwardIterator1 first1,ForwardIterator1 last1,ForwardIterator2 first2,ForwardIterator2 last2,Predicate pred){
  //计算迭代器之间的距离时需要知道距离类型
  search_impl(first1,last1,first2,last2,distance_type(first1),distance_type(first2),pred);
}

//序列B由n个v组成
template<class ForwardIterator,class Integer,class T>
ForwardIterator search_n(ForwardIterator first,ForwardIterator last,Integer count,const T& v){
  if(count <= 0)
    return first;
  //第一出现的位置
  first = find(first,last,v);
  ForwardIterator iter;
  Integer n;
  while(first != last){
    //进行滑动匹配
    iter = first;
    ++iter;
    n = count-1;
    while(iter != last && n != 0 && *iter == v){
      --n;
      ++iter;
    }
    if(n == 0)    //全部都匹配成功
      return first;
    else        //在接下来的序列中重新寻找起点匹配
      first = find(iter,last,v);
  }
  return last;
}

template<class ForwardIterator,class Integer,class T,class Predicate>
ForwardIterator search_n(ForwardIterator first,ForwardIterator last,Integer count,const T& v,Predicate pred){
  if(count <= 0)
    return first;
  //第一出现的位置
  first = find_if(first,last,v,pred);
  ForwardIterator iter;
  Integer n;
  while(first != last){
    //进行滑动匹配
    iter = first;
    ++iter;
    n = count-1;
    while(iter != last && n != 0 && pred(*iter,v)){
      --n;
      ++iter;
    }
    if(n == 0)    //全部都匹配成功
      return first;
    else        //在接下来的序列中重新寻找起点匹配
      first = find_of(iter,last,v,pred);
  }
  return last;
}

template<class ForwardIterator1,class ForwardIterator2,class T>
inline void iter_swap_impl(ForwardIterator1 a,ForwardIterator2 b,T*){

  T tmp = *a;
  *a = *b;
  *b = tmp;
}

//交换两个迭代器所指的元素
template<class ForwardIterator1,class ForwardIterator2>
inline void iter_swap(ForwardIterator1 a,ForwardIterator2 b){
      //需要定义临时变量 ---> 需要用到迭代器所指元的类型
      iter_swap_impl(a,b,value_type(a));
}

//交换两个迭代器所指范围
template<class ForwardIterator1,class ForwardIterator2>
ForwardIterator2 swap_ranges(ForwardIterator1 first1,ForwardIterator1 last1,ForwardIterator2 first2,ForwardIterator2 last2){
  //对范围内的每个迭代器进行元素交换
  for(;first1 != last1 && first2 != last2;++first1,++first2)
    iter_swap(first1,first2);
  return first2;
}

//将范围A的元素进行加工并转移到范围B中 --> 并没有对范围A进行改变
//返回iter -> B
template<class InputIterator,class OutputIterator,class UnaryOperation>
OutputIterator transform(InputIterator first,InputIterator last,OutputIterator result,UnaryOperation op){
    for(;first!=last;++first,++result)
      *result = op(*first);
    return result;
}
//将范围A和范围B的元素进行融合，在转移到范围C中
template<class InputIterator1,class InputIterator2,class OutputIterator,class BinaryOperation>
OutputIterator transform(InputIterator1 first1,InputIterator1 last1,InputIterator2 first2,InputIterator2 last2,OutputIterator result,BinaryOperation op){

    for(;first1!=last1 && first2 != last2;++first1,++first2,++result)
      *result = op(first1,first2);
    return result;
}


//将范围内的第一个a替换为b
template<class ForwardIterator,class T>
void replace(ForwardIterator first,ForwardIterator last,const T& old_value,const T& new_value){
  for(;first != last; ++first)
    if(*first == old_value)
      *first = new_value;
}
//将范围内满足条件的元素a替换为b
template<class ForwardIterator,class T,class Predicate>
void replace_if(ForwardIterator first,ForwardIterator last,const T& old_value,const T& new_value,Predicate pred){
  for(;first != last; ++first)
    if(pred(*first,old_value))
      *first = new_value;
}

//将范围A中的元素拷贝到范围B中，同时将a替换为b
template<class InputIterator,class OutputIterator,class T>
OutputIterator replace_copy(InputIterator first,InputIterator last,OutputIterator result,const T& old_value,const T& new_value){
  for(;first != last;++first,++result)
    result = (*first == old_value) ? new_value : *first;
  return result;
}

//条件谓词
template<class InputIterator,class OutputIterator,class Predicate,class T>
OutputIterator replace_copy(InputIterator first,InputIterator last,OutputIterator result,const T& new_value,Predicate pred){
  for(;first != last;++first,++result)
    result = (pred(*first) )? new_value : *first;
  return result;
}

//产生序列,存放于范围A中
template<class ForwardIterator,class Generator>
void generate(ForwardIterator first,ForwardIterator last,Generator gen){
  for(;first != last; ++first)
    *first = gen();
}

//产生n个元素的序列
template<class OutputIterator,class Size,class Generator>
void generate_n(OutputIterator first,Size n,Generator gen){
  for(;n>0;--n,++first)
    *first = gen();
  return first;
}

//将范围A拷贝到范围B,并移除其中的元素a --> 不拷贝a
template<class InputIterator,class OutputIterator,class T>
OutputIterator remove_copy(InputIterator first,InputIterator last,OutputIterator result,const T& value){
  for(;first != last; ++first){
    if(*first != value){
      *result = *first;
      ++result;
    }
  }
  return result;
}

//带有条件谓词版本
template<class InputIterator,class OutputIterator,class Predicate>
OutputIterator remove_copy(InputIterator first,InputIterator last,OutputIterator result,Predicate pred){
  for(;first != last; ++first){
    if(pred(*first)){
      *result = *first;
      ++result;
    }
  }
}

//移除范围A中的元素a
template<class ForwardIterator,class T>
ForwardIterator remove(ForwardIterator first,ForwardIterator last,const T& value){
  //元素value第一次出现的位置
  first = find(first,last,value);
  ForwardIterator next = first;
  ++next;
  //区间的从前向后的拷贝覆盖
  return (first == last)? first : remove_copy(next,last,first,first,value);
}

template<class ForwardIterator,class Predicate>
ForwardIterator remove(ForwardIterator first,ForwardIterator last,Predicate pred){
  //元素第一次满足条件出现的位置
  first = find_if(first,last,pred);
  ForwardIterator next = first;
  ++next;
  //区间的从前向后的拷贝覆盖
  return (first == last)? first : remove_copy_if(next,last,first,first,pred);
}

template<class InputIterator,class OutputIterator,class T>
OutputIterator unique_copy_impl(InputIterator first,InputIterator last,OutputIterator result,T*){
  T value = *first;    //存储前一个值
  *result = value;
  while(++first != last){
    if(*first != value){
      value = *first;         //不重复的值
      *++result = value;
    }
  }
 return ++result;
}

template<class InputIterator,class ForwardIterator>
inline ForwardIterator unique_copy_dispatch(InputIterator first,InputIterator last,ForwardIterator result,forward_iterator_tag){
    *result = *first;
    while(++first != last)
      if(*result !=*first)
        *++result = *first;
    return ++result;
}

template<class InputIterator,class OutputIterator>
inline OutputIterator unique_copy_dispatch(InputIterator first,InputIterator last,OutputIterator result,output_iterator_tag){
  //要建立临时变量 ---> 需要知道迭代器所指对象的类型
  return unique_copy_impl(first,last,result,value_type(first));
}


//将范围A不相邻重复的拷贝到范围B中
template<class InputIterator,class OutputIterator>
inline OutputIterator unique_copy(InputIterator first,InputIterator last,OutputIterator result){
  if(first == last)
    return;
  //根据result迭代器的类型进行分发
  return unique_copy_dispatch(first,last,result,iterator_category(result));
}

//output_iterator为write_only -> 不能读取 --> *result不行
template<class InputIterator,class OutputIterator,class T,class BinaryPrediacte >
OutputIterator unique_copy_impl(InputIterator first,InputIterator last,OutputIterator result,T*,BinaryPrediacte binary_pred){
  T value = *first;    //存储前一个值
  *result = value;
  while(++first != last){
    if(binary_pred(*first,value)){
      value = *first;         //不相邻重复的值
      *++result = value;
    }
  }
 return ++result;
}

template<class InputIterator,class ForwardIterator,class BinaryPrediacte>
inline ForwardIterator unique_copy_dispatch(InputIterator first,InputIterator last,ForwardIterator result,forward_iterator_tag,BinaryPrediacte  binary_pred){
    *result = *first;
    while(++first != last)
      if(binary_pred(*result,*first))
        *++result = *first;
    return ++result;
}

template<class InputIterator,class OutputIterator,class BinaryPrediacte >
inline OutputIterator unique_copy_dispatch(InputIterator first,InputIterator last,OutputIterator result,output_iterator_tag,BinaryPrediacte binary_pred){
  //要建立临时变量 ---> 需要知道迭代器所指对象的类型
  return unique_copy_impl(first,last,result,value_type(first),binary_pred);
}


//将范围A不相邻重复的拷贝到范围B中
template<class InputIterator,class OutputIterator,class BinaryPrediacte>
inline OutputIterator unique_copy(InputIterator first,InputIterator last,OutputIterator result,BinaryPrediacte binary_pred){
  if(first == last)
    return;
  //根据result迭代器的类型进行分发
  return unique_copy_dispatch(first,last,result,iterator_category(result),binary_pred);
}

//覆盖掉范围内相邻重复的元素
template<class ForwardIterator>
ForwardIterator unique(ForwardIterator first,ForwardIterator last){
  //第一对相邻重复的元素位置
  first = adjacent_find(first,last);
  //覆盖
  return unique_copy(first,last,first);
}

template<class ForwardIterator,class Predicate>
ForwardIterator unique(ForwardIterator first,ForwardIterator last,Predicate pred){
  //第一对相邻重复的元素位置
  first = adjacent_find(first,last,pred);
  //覆盖
  return unique_copy(first,last,first,pred);
}


//双向迭代器不支持<
template<class BidirectionalIterator>
void reverse(BidirectionalIterator first,BidirectionalIterator last,bidirectional_iterator_tag){
  while(true)
    if(first == last || first == --last)
      return ;
    else 
      iter_swap(first++,last);
}

template<class BidirectionalIterator>
void reverse(BidirectionalIterator first,BidirectionalIterator last,random_access_iterator_tag){
  //随机迭代器可以进行<
  while (first < last) iter_swap(first++, --last);
}

//逆置范围内的元素
template<class BidirectionalIterator>
void reverse(BidirectionalIterator first,BidirectionalIterator last){
  reverse_impl(first,last,iterator_category(first));
}

//将范围A中的元素反向拷贝到B中
template<class BidirectionalIterator,class OutputIterator> 
OutputIterator reverse_copy(BidirectionalIterator first,BidirectionalIterator last,OutputIterator result){
  while(first != last){
    *result = *--last;
    ++result;
  }
  return result;
}

//对于前向迭代器，其只能++
template<class ForwardIterator,class Distance>
void rotate_dispatch(ForwardIterator first,ForwardIterator middle,ForwardIterator last,Distance*,forward_iterator_tag){
  for(auto i = middle;;){
      //平行向前交换
      iter_swap(first,i);
      ++first;
      ++i;
      //查看是否有某一段已经到了终点
      //前半部分到达终点
      //后半部分剩余部分再分成两段，继续调整 ---> 类似于递归
      if(first == middle){
        if(i == last)
          return;
        middle = i;
      }else if(i == last){
      //后半部分达到终点
      //前半部分的剩余部分再分成两段
      i = middle;
      }
  }
}
///双向迭代器可以调用reverse
template<class BidirectionalIterator,class Distance>
void rotate_dispatch(BidirectionalIterator first,BidirectionalIterator middle,BidirectionalIterator last,Distance*,bidirectional_iterator_tag){
  //逆转前半部分
  reverse(first,middle);
  //逆转后半部分
  reverse(middle,last);
  //逆转全部
  reverse(first,last);
}

//利用辗转相除法得到最大公约数
template<class Distance>
Distance gcd(Distance m,Distance n){
      Distance r;
      while(n != 0){
        r = m % n;
        m = n;
        n = r;
      }
      return m;
}

//It's amazaing 
//可能运用了某些数学定理
//随机迭代器可以+n 
template <class RandomAccessIterator, class Distance, class T>
void rotate_cycle(RandomAccessIterator first, RandomAccessIterator last,
                    RandomAccessIterator initial, Distance shift, T*) {
      T value = *initial;
      auto p1 = initial;
      auto p2 = p1+shift;
      while(p2 != p1){
        *p1 = *p2;
         p1 = p2;
         if((last-p2) > shift)
           p2 += shift;
         else 
           p2 = first+(shift-(last-p2));
      }
      *p1 = value;
}
template<class RandomAccessIter,class Distance>
void rotate_dispatch(RandomAccessIter first,RandomAccessIter middle,RandomAccessIter last,Distance*,random_access_iterator_tag){
  //最大公约数
  Distance n = gcd(last-first,middle-first);
  while(n--)
    rotate_recycle(first,last,first+n,middle-first,value_type(first));
}
//将[first,midle),[midle,last)进行交换
template<class ForwardIterator>
inline void rotate(ForwardIterator first,ForwardIterator middle,ForwardIterator last){
  if(first == middle || middle == last) 
    return;
  //根据迭代器分类进行分配
  rotate_dispatch(first,middle,last,distance_type(first),iterator_category(first));
}

//将[first,mid),[mid,last)翻转后拷贝到范围B中
template<class ForwardIterator,class OutputIterator>
OutputIterator rotate_copy(ForwardIterator first,ForwardIterator mid,ForwardIterator last,OutputIterator result){
  return copy(first,mid,copy(mid,last,result));
}

//区间划分，让满足条件A的元素位于区间后面
template<class BidirectionalIterator,class Predicate>
BidirectionalIterator partition(BidirectionalIterator first,BidirectionalIterator last,Predicate pred){
  //从前向后找到第一个满足条件的元素
  //从后向前找到第一个不满足条件的元素
  //二者进行交换
  while(true){
    while(true)
      if(first == last)
        return first;
      else if (pred(*first))
        ++first;
      else break;
    --last;
    while(true)
      if(first == last)
        return first;
      else if(!pred(*last))
        --last;
      else break;
    iter_swap(first,last);
    ++first;
  }
}
template <class ForwardIterator, class Predicate, class Distance>
ForwardIterator inplace_stable_partition(ForwardIterator first,ForwardIterator last,Predicate pred, Distance len) {
  
  //分而治之
  if(len == 1)
    return pred(*first) ? last : first;
  ForwardIterator mid = first;
  advance(mid,len/2);
  ForwardIterator first_cut =inplace_stable_partition(first,mid,pred,len/2);
  ForwardIterator second_cut=inplace_stable_partition(mid,last,pred,len-len/2);
  rotate(first_cut,mid,second_cut);
  len = 0;
  distance(mid,second_cut,len);
  advance(first_cut,len);
  return first_cut;
}

template <class ForwardIterator, class Pointer, class Predicate, 
          class Distance>
          ForwardIterator stable_partition_adaptive(ForwardIterator first,ForwardIterator last,Predicate pred, Distance len,Pointer buffer,Distance buffer_size) {

            if(len<=buffer_size){ //分配了足够的空间
              ForwardIterator result1 = first;
              Pointer result2= buffer;
              for(;first != last; ++first){
                if(pred(*first)){
                  *result1 = *first;
                  ++result1;
                }else{
                  //不满足条件的元素先放在buffer中
                  *result2 = *first;
                  ++result2;
                }
              }
                copy(buffer,result2,result1);
                return result1;
            }else{
              //没有足够的空间，分而治之
              ForwardIterator mid = first;
              advance(mid,len/2);
              ForwardIterator first_cut = stable_partition_adaptive(first,mid,pred,len/2,buffer,buffer_size);
              ForwardIterator second_cut = stable_partition_adaptive(mid,last,pred,len-len/2,buffer,buffer_size);
              rotate(first_cut,mid,second_cut);
              distance(first_cut,len);
              advance(first_cut,len);
              return first_cut;
            }
          }


template<class ForwardIterator,class Predicate,class T,class Distance>
inline ForwardIterator stable_partition_impl(ForwardIterator first,ForwardIterator last,Predicate pred,T*,Distance*){
    
  //构建缓冲区
  temporary_buffer<ForwardIterator,T> buf(first,last);
  if(buf.size > 0)
    return stable_partition_adaptive(first,last,pred,Distance(buf.requested_size()),buf.begin(),buf.size());
  else 
    //缓冲区构建失败
    
    return inplace_stable_partition(first,last,pred,Distance(buf.requested_size()));

}
//划分区间并保持元素的相对次序
template<class ForwardIterator,class Predicate>
inline ForwardIterator stable_partition(ForwardIterator first,ForwardIterator last,Predicate pred){
  if(first == last)
    return first;
  else 
    return stable_partition_impl(first,last,pred,value_type(first),distance_type(first));
}



const int threshold = 16;


//找到2^K <= n 的最大值K 当n=8时,K=3
template<class Size> 
inline Size max_recursive(Size n){
  Size k;
  for(k=0;n>1;n>>=1)
    ++k;
  return k;
}


template<class RandomAccessIterator,class T>
void insert_sort_impl(RandomAccessIterator first,RandomAccessIterator last,T*){
  if(first == last)
    return;
  T tmp;
  RandomAccessIterator cur,next;
  //i表示每次要插入的元素
  //区间[first,i) ----> 已经排好序
  for(auto i=first+1; i != last;++i){
     tmp = *i;
     if(tmp < *first){  //tmp需要插入到最前面 --> 整体向后移
       copy_backward(first,i,i+1);
       *first = tmp;
     }else{      //需要找到正确的插入位置
       cur = next = i;
       --cur;
        while(*cur > tmp){  //若tmp比较下，则向前找插入位置
          *next = *cur;
           next = cur;
           --cur;
        }
        *next = tmp;
     }
  }
}

template<class RandomAccessIter>
void insert_sort(RandomAccessIter first,RandomAccessIter last){
    insert_sort_impl(first,last,value_type(first));
}

//返回三个值中第二大的
template<class T>
inline const T& median(const T&a,const T&b,const T&c){
  if(a<b){
    if(b<c)
      return b;
    else if(a<c)
      return c;
    else return a;
  }else if(a<c)
    return a;
  else if(b<c)
    return c;
  else 
    return b;
}

template <class RandomAccessIterator, class T>
void partial_sort_impl(RandomAccessIterator first, RandomAccessIterator middle,RandomAccessIterator last, T*) {
    //先将[first,middle]建成堆
    make_heap(first,middle);  //最大值位于堆顶
    //比较[middle,last]的元素x与堆顶元素y
    //若x小于<y,则将x纳入堆中,将y踢出并重新调整堆序性
    for(auto i = middle;i<last;i++){
      if(*i < *first){
        iter_swap(i,first);
        adjust_heap(first,middle);
      }
    }
    sort_heap(first,middle);
}
//采用堆排序，将middle-first个较小的元素放到[first,middle]中,其余元素放在[middle,last],并不保证其顺序性
template<class RandomAccessIter,class T>
void partial_sort(RandomAccessIter first,RandomAccessIter middle,RandomAccessIter last){
    partial_sort_impl(first,middle,last,value_type(first));
}


template <class RandomAccessIterator, class T,class Compare>
void partial_sort_impl(RandomAccessIterator first, RandomAccessIterator middle,RandomAccessIterator last, T*,Compare comp) {
    //先将[first,middle]建成堆
    make_heap(first,middle,comp);  //最大值位于堆顶
    //比较[middle,last]的元素x与堆顶元素y
   //若x小于<y,则将x纳入堆中,将y踢出并重新调整堆序性
    for(auto i = middle;i<last;i++){
      if(comp(*i,*first)){
        iter_swap(i,first);
        adjust_heap(first,middle,comp);
      }
    }
    sort_heap(first,middle,comp);
}
//带比较器版本
template<class RandomAccessIter,class T,class Compare>
void partial_sort(RandomAccessIter first,RandomAccessIter middle,RandomAccessIter last,Compare comp){
    partial_sort_impl(first,middle,last,value_type(first),comp);
}


template<class RandomAccessIter,class T>
RandomAccessIter unguarded_partition(RandomAccessIter first,RandomAccessIter last,T pivot){
  //从左向右找到第一个>=pivot的元素 ---> 让小于pivot的元素位于左边
  //从右向左找到第一个<=pivot的元素 ---> 让大于pivot的元素位于右边
  //二者交换   重复上述过程，直到区间[first,last]不再有效
  while(true){
    while(*first < pivot)
      ++first;
    --last;
    while(*last > pivot)
      --last;
    //区间有效性判断  ---> first >= last 
    if(!(first < last))
      return first;
    iter_swap(first,last);
    ++first;
  }
}

template<class RandomAccessIter,class T,class Size> 
void quick_sort(RandomAccessIter first,RandomAccessIter last,T*,Size recursive_limit){

  while(last-first > threshold){  //超过阈值时才采用此方案

    if(recursive_limit == 0){ //已达到最大递归深度，采取堆排序
      partial_sort(first,last,last);
      return;
    }

    //区间划分 ---> 没有进行边界检查
    //取首中尾三个的中间值作为划分
    RandomAccessIter cut = unguarded_partition(first,last,T(meidan(*first,*(first+(last-first)/2),*(last-1))));
    //递归调用
    quick_sort(cut,last,value_type(first),recursive_limit);
    //后半部分处理结束，转向前半部分
    last = cut;
  }
}

template<class RandomAccessIter>
inline void sort(RandomAccessIter first,RandomAccessIter last){
  if(first != last){
    quick_sort(first,last,value_type(first),max_recursive(last-first)*2);
   //max_recursive quick sort的最大递归层次 ---> 前半部分，后半部分
    //通过quick_sort对大范围进行处理好以后，接下来采用插入排序
    insert_sort(first,last);
  }
}


//自带比较器版本
template<class RandomAccessIterator,class T,class Compare>
void insert_sort_impl(RandomAccessIterator first,RandomAccessIterator last,T*,Compare comp){
  if(first == last)
    return;
  T tmp;
  RandomAccessIterator cur,next;
  for(auto i=first+1; i != last;++i){
     tmp = *i;
     if(comp(tmp,first)){  
       copy_backward(first,i,i+1);
       *first = tmp;
     }else{      
       cur = next = i;
       --cur;
        while(comp(tmp,*cur)){ 
          *next = *cur;
           next = cur;
           --cur;
        }
        *next = tmp;
     }
  }
}

template<class RandomAccessIter,class Compare>
void insert_sort(RandomAccessIter first,RandomAccessIter last,Compare comp){
    insert_sort_impl(first,last,value_type(first),comp);
}

template<class T,class Compare>
inline const T& median(const T&a,const T&b,const T&c,Compare comp){
  if(comp(a,b)){
    if(comp(b,c))
      return b;
    else if(comp(a,c))
      return c;
    else return a;
  }else if(comp(a,c))
    return a;
  else if(comp(b,c))
    return c;
  else 
    return b;
}



template<class RandomAccessIter,class T,class Compare>
RandomAccessIter unguarded_partition(RandomAccessIter first,RandomAccessIter last,T pivot,Compare comp){
  while(true){
    while(comp(*first, pivot))
      ++first;
    --last;
    while(comp(pivot,*last))
      --last;
    if(!(first < last))
      return first;
    iter_swap(first,last);
    ++first;
  }
}

template<class RandomAccessIter,class T,class Size,class Compare> 
void quick_sort(RandomAccessIter first,RandomAccessIter last,T*,Size recursive_limit,Compare comp){

  while(last-first > threshold){  

    if(recursive_limit == 0){ 
      partial_sort(first,last,last,comp);
      return;
    }
    RandomAccessIter cut = unguarded_partition(first,last,T(meidan(*first,*(first+(last-first)/2),*(last-1))),comp);
    quick_sort(cut,last,value_type(first),recursive_limit,comp);
    last = cut;
  }
}

template<class RandomAccessIter,class Compare>
inline void sort(RandomAccessIter first,RandomAccessIter last,Compare comp){
  if(first != last){
    quick_sort(first,last,value_type(first),max_recursive(last-first)*2,comp);
    insert_sort(first,last,comp);
  }
}



//范围A和范围B升排列，将二者融合并输出到范围c中
template<class InputIterator1,class InputIterator2,class OutputIterator>
OutputIterator merge(InputIterator1 first1,InputIterator1 last1,InputIterator2 first2,InputIterator2 last2,OutputIterator result){

  while(first1 != last1 && first2 != last2){
    if(*first1 < *first2){
      *result = *first1;
      ++first1;
    }else{
      *result = *first2;
      ++first2;
    }
    ++result;
  }
    return copy(first2,last2,copy(first1,last1,result));
}


template<class RandomAccessIter1,class RandomAccessIter2,class Distance>
void chunk_merge_loop(RandomAccessIter1 first,RandomAccessIter1 last,RandomAccessIter2 result,Distance chunk_size){
    //对每两块调用merge进行合并，合并结果存放于result中
    Distance two_chunk = 2*chunk_size;
    while(last-first >= two_chunk){
      result = merge(first,first+chunk_size,first+chunk_size,first+two_chunk,result);
      first += two_chunk;
    }
    //处理最后的一小块
    chunk_size = mind(Distance(last-first),chunk_size);
    merge(first,first+chunk_size,first+chunk_size,last,result);
}

template<class BidirectionalIterator1,class BidirectionalIterator2,class BidirectionalIterator3>
BidirectionalIterator3 merge_backward(BidirectionalIterator1 first1,BidirectionalIterator1 last1,BidirectionalIterator2 first2,BidirectionalIterator2 last2,BidirectionalIterator3 result){
   //特殊情况考虑
    if(first1 == last1)
      return copy_backward(first2,last2,result);
    if(first2 == last2)
      return copy_backward(first1,last1,result);
    --last1;
    --last2;
    while(true){
      if(*last2 < *last1){
        *--result = *last1;
        if(last1 == first1) //要满足[)
          return copy_backward(first2,++last2,result);
        --last1;
      }else {
        *--result = last2;
        if(last2 == first2)
          return copy_backward(first1,++last1,result);
        --last2;
      }
    }
}

//也许可以用缓冲区辅助性地rotate --> 根据缓冲区大小自适应地选择
template<class BidirectionalIterator1,class BidirectionalIterator2,class Distance>
BidirectionalIterator1 rotate_adaptive(BidirectionalIterator1 first,BidirectionalIterator1 mid_iter,BidirectionalIterator1 last,Distance len1,Distance len2,BidirectionalIterator2 buffer,Distance buffer_size){
      BidirectionalIterator2 buffer_end;
      //序列2长度小于buffer
      //则将序列2拷贝到buffer中
      //将序列1copy_backward到序列2中
      //再讲buffer拷贝到序列1中
      //类似swap
      if(len1 > len2 && len2 <= buffer_size){
        buffer_end = copy(mid_iter,last,buffer);
        copy_backward(first,mid_iter,last);
        return copy(buffer,buffer,first);
      }else if(len1 <= buffer_size){
        buffer_end = copy(first,mid_iter,last);
        copy(mid_iter,last,first);
        return copy_backward(buffer,buffer_end,last);
      }else{  //不能再借助缓冲区
        rotate(first,mid_iter,last);
        advance(first,len2);
        return first;
      }
}

template<class BidirectionalIterator,class Distance,class Pointer>
void merge_adaptive(BidirectionalIterator first,BidirectionalIterator mid_iter,BidirectionalIterator last,Distance len1,Distance len2,Pointer buffer,Distance buffer_size){
  Pointer buffer_end;
  if(len1 <= len2 && len1 <=buffer_size){ //将序列1的元素拷贝到缓冲区中.再将缓冲区和序列2合并，存放回序列中
     buffer_end = copy(first,mid_iter,buffer);
     merge(buffer,buffer_end,mid_iter,last,first);
  }else if(len2 <= buffer_size){
    buffer_end = copy(mid_iter,last,buffer);
    //从后向前merge
    merge_backward(first,mid_iter,buffer,buffer_end,last);
  }else{
    //采用merge_without_buffer的方式
    BidirectionalIterator first_cut = first;
    BidirectionalIterator second_cut = mid_iter;
    Distance l1=0,l2=0;
    if(len1 > len2){
      l1 = len1/2;
      advance(first_cut,l1);
      second_cut = lower_bound(mid_iter,last,*first_cut);
      distance(mid_iter,second_cut,l2);
    }else{
      l2 = len2/2;
      advance(second_cut,l2);
      first_cut = upper_bound(first,mid_iter,*second_cut);
      distance(first,first_cut,l1);
    }
    BidirectionalIterator new_mid = rotate_adaptive(first_cut,mid_iter,second_cut,len1-l1);
    //递归缩小区间
    merge_adaptive(first, first_cut, new_mid, l1, l2, buffer,buffer_size);
    merge_adaptive(new_mid, second_cut, last, len1 - l1,len2-l2,buffer,buffer_size);
  }
}
const size_t CHUNK_SIZE= 7;


template<class RandomAccessIter,class Distance>
void chunk_insert_sort(RandomAccessIter first,RandomAccessIter last,Distance chunk_size){
  //对小块内的元素进行插入排序--->降低copy_backward的开销
  while(last - first >= chunk_size){
    insert_sort(first,first+chunk_size);
    first+=chunk_size;
  }
  insert_sort(first,last);
}
template<class RandomAccessIter,class Pointer, class Distance>
void merge_sort_with_buffer(RandomAccessIter first,RandomAccessIter last,Pointer buffer,Distance*){
  //元素个数
  Distance len = last - first;
  Pointer buffer_last = buffer+last;
  Distance chunk_size= chunk_size;
  //将区间的内元素分块，对每一块进行插入排序
  chunk_insert_sort(first,last,chunk_size);
  while(chunk_size < len){
      //先将序列中的元素按小块两两合并存放在buffer中
      chunk_merge_loop(first,last,buffer,chunk_size);
      chunk_size *= 2;
      //再buffer中的元素按照较大块两两合并存放在序列中农
      chunk_merge_loop(buffer,buffer_last,first,chunk_size);
      chunk_size*=2;
      //当块的大小达到了序列大小，则表明合并结束
  }
}
template<class RandomAccessIter,class Pointer,class Distance>
void stable_sort_adaptive(RandomAccessIter first,RandomAccessIter last,Pointer buffer,Distance buffer_size){
  Distance mid = (last-first+1)/2;
  RandomAccessIter mid_iter = first+mid;
  
  if(mid > buffer_size){ //缓冲区较小
      //序列区间缩小
      stable_sort_adaptive(first,mid_iter,buffer,buffer_size);
      stable_sort_adaptive(mid_iter,last,buffer,buffer_size);
  }else{
    //相对于当前的序列区间，缓冲区大小足够，借助缓冲区进行合并
    merge_sort_with_buffer(first,mid_iter,buffer,(Distance*)0);
    merge_sort_with_buffer(mid_iter,last,buffer,(Distance*)0);
  }
  //最后再对整体的区间进行合并
  merge_adaptive(first,mid_iter,last,Distance(mid_iter - first),Distance(last-mid_iter),buffer,buffer_size);
}


template<class BidirectionalIterator,class Distance>
void merge_without_buffer(BidirectionalIterator first,BidirectionalIterator mid,BidirectionalIterator last,Distance len1,Distance len2){
    
  //边界情况考虑
  if(len1 == 0 || len2 == 0)
    return;
  if(len1 + len2 == 2){
    if(*mid < *first)
      iter_swap(first,mid);
    return;
  }
  
  //思想:序列1[first,mid)和2[mid,last)都是升序排列
  //若序列1比较长，则在序列1中取一临界点，设为x1
  //在序列2中找到第一个>=*x1的元素,下标设为x2
  //[x1,mid) 和 [mid,x2)都是升序排列
  //[mid,x2)比[x1,mid)更加贴近x1前面的元素
  //而[x1,mid)比[midx,x2)更加贴近x2后面的元素
  //故对二者进行交换
  //故有:[first,mid),[mid,x2]升序排列 ---> 新的序列1
  //     [x1,mid),[x2,last) 升序排列 ---> 新的序列2
  //     进行递归调用
  //     直到递归临界为止

  BidirectionalIterator first_cut = first;
  BidirectionalIterator second_cut =  mid;
  Distance l1 = 0;
  Distance l2 = 0;
  if(len1 > len2){
    l1 = len1 / 2;
    advance(first_cut,l1);
    //在序列2中找到第一个>=*first_cut的元素
    second_cut = lower_bound(mid,last,*first_cut);
    distance(mid,second_cut,l2);
  }else{
    l2 = len2/2;
    advance(second_cut,l2);  
    //在序列1中找到第一个>*second_cut的元素
    first_cut = upper_bound(first,mid,*second_cut);
    distance(first,first_cut,l1);
  }
  rotate(first_cut,mid,second_cut);
  BidirectionalIterator new_mid = first_cut;
  advance(new_mid,l2);
  merge_without_buffer(first,first_cut,new_mid,l1,l2);
  merge_without_buffer(new_mid,second_cut,last,len1-l1,len2-l2);
}
    
template<class RandomAccessIter>
void inplace_stable_sort(RandomAccessIter first,RandomAccessIter last){
      
  if(last - first){       //若序列中的数量较少，则采用插入排序，对该小序列进行排序，稍后在合并
    insert_sort(first,last);
    return;
  }
  //二分区间,递归调用
  RandomAccessIter mid = first+(last-first)/2;
  inplace_stable_sort(first,mid);
  inplace_stable_sort(mid,last);
  //合并
  merge_without_buffer(first,mid,last,mid-first,last-mid);
}

template<class RandomAccessIter,class Distance,class T>
inline void stable_sort__(RandomAccessIter first,RandomAccessIter last,Distance*,T*){
      //分配缓冲区
      temporary_buffer<RandomAccessIter,T> buf(first,last);
      if(buf.begin() == nullptr) //分配失败
        inplace_stable_sort(first,last);
      else 
        stable_sort_adaptive(first,last,buf.begin(),Distance(buf.size()));
}

//稳定的排序 ---> 元素相对次序不会改变
template<class RandomAccessIter>
inline void stable_sort(RandomAccessIter first,RandomAccessIter last){
    //需要定义距离类型和对象类型的变量
    stable_sort__(first,last,distance_type(first),value_type(first));
}


//范围A和范围B升排列，将二者融合并输出到范围c中
template<class InputIterator1,class InputIterator2,class OutputIterator,class Compare>
OutputIterator merge(InputIterator1 first1,InputIterator1 last1,InputIterator2 first2,InputIterator2 last2,OutputIterator result,Compare comp){

  while(first1 != last1 && first2 != last2){
    if(comp(*first1 ,*first2)){
      *result = *first1;
      ++first1;
    }else{
      *result = *first2;
      ++first2;
    }
    ++result;
  }
    return copy(first2,last2,copy(first1,last1,result));
}


template<class RandomAccessIter1,class RandomAccessIter2,class Distance,class Compare>
void chunk_merge_loop(RandomAccessIter1 first,RandomAccessIter1 last,RandomAccessIter2 result,Distance chunk_size,Compare comp){
    //对每两块调用merge进行合并，合并结果存放于result中
    Distance two_chunk = 2*chunk_size;
    while(last-first >= two_chunk){
      result = merge(first,first+chunk_size,first+chunk_size,first+two_chunk,result,comp);
      first += two_chunk;
    }
    //处理最后的一小块
    chunk_size = std::min(Distance(last-first),chunk_size);
    merge(first,first+chunk_size,first+chunk_size,last,result,comp);
}


//类似merge,只不过是从后向前merge
template<class BidirectionalIterator1,class BidirectionalIterator2,class BidirectionalIterator3,class Compare>
BidirectionalIterator3 merge_backward(BidirectionalIterator1 first1,BidirectionalIterator1 last1,BidirectionalIterator2 first2,BidirectionalIterator2 last2,BidirectionalIterator3 result,Compare comp){
   //特殊情况考虑
    if(first1 == last1)
      return copy_backward(first2,last2,result,comp);
    if(first2 == last2)
      return copy_backward(first1,last1,result,comp);
    --last1;
    --last2;
    while(true){
      if(comp(*last2, *last1)){
        *--result = *last1;
        if(last1 == first1) //要满足[)
          return copy_backward(first2,++last2,result,comp);
        --last1;
      }else {
        *--result = last2;
        if(last2 == first2)
          return copy_backward(first1,++last1,result,comp);
        --last2;
      }
    }
}


template<class BidirectionalIterator,class Distance,class Pointer,class Compare>
void merge_adaptive(BidirectionalIterator first,BidirectionalIterator mid_iter,BidirectionalIterator last,Distance len1,Distance len2,Pointer buffer,Distance buffer_size,Compare comp){
  Pointer buffer_end;
  if(len1 <= len2 && len1 <=buffer_size){ //将序列1的元素拷贝到缓冲区中.再将缓冲区和序列2合并，存放回序列中
     buffer_end = copy(first,mid_iter,buffer);
     merge(buffer,buffer_end,mid_iter,last,first,comp);
  }else if(len2 <= buffer_size){
    buffer_end = copy(mid_iter,last,buffer);
    //从后向前merge
    merge_backward(first,mid_iter,buffer,buffer_end,last,comp);
  }else{
    //采用merge_without_buffer的方式
    BidirectionalIterator first_cut = first;
    BidirectionalIterator second_cut = mid_iter;
    Distance l1=0,l2=0;
    if(len1 > len2){
      l1 = len1/2;
      advance(first_cut,l1);
      second_cut = lower_bound(mid_iter,last,*first_cut,comp);
      distance(mid_iter,second_cut,l2);
    }else{
      l2 = len2/2;
      advance(second_cut,l2);
      first_cut = upper_bound(first,mid_iter,*second_cut,comp);
      distance(first,first_cut,l1);
    }
    BidirectionalIterator new_mid = rotate_adaptive(first_cut,mid_iter,second_cut,len1-l1);
    //递归缩小区间
    merge_adaptive(first, first_cut, new_mid, l1, l2, buffer,buffer_size,comp);
    merge_adaptive(new_mid, second_cut, last, len1 - l1,len2-l2,buffer,buffer_size,comp);
  }
}


template<class RandomAccessIter,class Distance,class Compare>
void chunk_insert_sort(RandomAccessIter first,RandomAccessIter last,Distance chunk_size,Compare comp){
  //对小块内的元素进行插入排序--->降低copy_backward的开销
  while(last - first >= chunk_size){
    insert_sort(first,first+chunk_size,comp);
    first+=chunk_size;
  }
  insert_sort(first,last,comp);
}
template<class RandomAccessIter,class Pointer, class Distance,class Compare >
void merge_sort_with_buffer(RandomAccessIter first,RandomAccessIter last,Pointer buffer,Distance*,Compare comp){
  Distance len = last - first;
  Pointer buffer_last = buffer+last;
  Distance chunk_size= chunk_size;
  chunk_insert_sort(first,last,chunk_size,comp);
  while(chunk_size < len){
      chunk_merge_loop(first,last,buffer,chunk_size,comp);
      chunk_size *= 2;
      chunk_merge_loop(buffer,buffer_last,first,chunk_size,comp);
      chunk_size*=2;
  }
}
template<class RandomAccessIter,class Pointer,class Distance,class Compare>
void stable_sort_adaptive(RandomAccessIter first,RandomAccessIter last,Pointer buffer,Distance buffer_size,Compare comp){
  Distance mid = (last-first+1)/2;
  RandomAccessIter mid_iter = first+mid;
  
  if(mid > buffer_size){ //缓冲区较小
      //序列区间缩小
      stable_sort_adaptive(first,mid_iter,buffer,buffer_size,comp);
      stable_sort_adaptive(mid_iter,last,buffer,buffer_size,comp);
  }else{
    //相对于当前的序列区间，缓冲区大小足够，借助缓冲区进行合并
    merge_sort_with_buffer(first,mid_iter,buffer,(Distance*)0,comp);
    merge_sort_with_buffer(mid_iter,last,buffer,(Distance*)0,comp);
  }
  //最后再对整体的区间进行合并
  merge_adaptive(first,mid_iter,last,Distance(mid_iter - first),Distance(last-mid_iter),buffer,buffer_size,comp);
}


template<class BidirectionalIterator,class Distance,class Compare>
void merge_without_buffer(BidirectionalIterator first,BidirectionalIterator mid,BidirectionalIterator last,Distance len1,Distance len2,Compare comp){
    
  //边界情况考虑
  if(len1 == 0 || len2 == 0)
    return;
  if(len1 + len2 == 2){
    if(comp(*mid,*first))
      iter_swap(first,mid);
    return;
  }

  BidirectionalIterator first_cut = first;
  BidirectionalIterator second_cut =  mid;
  Distance l1 = 0;
  Distance l2 = 0;
  if(len1 > len2){
    l1 = len1 / 2;
    advance(first_cut,1);
    //在序列2中找到第一个>=*first_cut的元素
    second_cut = lower_bound(mid,last,*first_cut);
    distance(mid,second_cut,l2);
  }else{
    l2 = len2/2;
    advance(second_cut,l2);  
    //在序列1中找到第一个>*second_cut的元素
    first_cut = upper_bound(first,mid,*second_cut);
    distance(first,first_cut,l1);
  }
  rotate(first_cut,mid,second_cut);
  BidirectionalIterator new_mid = first_cut;
  advance(new_mid,l2);
  merge_without_buffer(first,first_cut,new_mid,l1,l2,comp);
  merge_without_buffer(new_mid,second_cut,last,len1-l1,len2-l2,comp);
}
    
template<class RandomAccessIter,class Compare>
void inplace_stable_sort(RandomAccessIter first,RandomAccessIter last,Compare comp){
      
  if(last - first){       //若序列中的数量较少，则采用插入排序，对该小序列进行排序，稍后在合并
    insert_sort(first,last,comp);
    return;
  }
  //二分区间,递归调用
  RandomAccessIter mid = first+(last-first)/2;
  inplace_stable_sort(first,mid,comp);
  inplace_stable_sort(mid,last,comp);
  //合并
  merge_without_buffer(first,mid,last,mid-first,last-mid,comp);
}

template<class RandomAccessIter,class Distance,class T,class Compare>
inline void stable_sort__(RandomAccessIter first,RandomAccessIter last,Distance*,T*,Compare comp){
      temporary_buffer<RandomAccessIter,T> buf(first,last);
      if(buf.begin() == nullptr) //分配失败
        inplace_stable_sort(first,last,comp);
      else 
        stable_sort_adaptive(first,last,buf.begin(),Distance(buf.size()),comp);
}


//带有比较器版本
template<class RandomAccessIter,class Compare>
inline void stable_sort(RandomAccessIter first,RandomAccessIter last,Compare comp){
    stable_sort__(first,last,distance_type(first),value_type(first),comp);
}
//判断序列是否是排序(升序)的
template<class ForwardIterator>
bool is_sorted(ForwardIterator first,ForwardIterator last){
  if(first == last)
    return true;
  auto next = first;
  for(;next != last;first = next,++next)
    if(*next < *first)
      return false;
   return true;
}

template<class RandomAccessIter,class T>
inline void nth_element__(RandomAccessIter first,RandomAccessIter nth,RandomAccessIter last,T*){
  RandomAccessIter cut;
  //元素个数大于3时
  while(last -first > 3){
    cut = unguarded_partition(first,last,T(median(*first,*(first+(last-first)/2),*(last-1))));
    //进入右边
    if(cut <= nth)
      first = cut;
    //进入左边
    else 
      last = cut;
  }
  //划分到足够小，直接排序
  insert_sort(first,last);
}
//重新排列[first,last]
//使得所有<*nth的元素都位于左边,大于>*th的元素位于右边
//nth代表排序后第n大元素的位置
//利用快排思想
template<class RandomAccessIter>
inline void nth_element(RandomAccessIter first,RandomAccessIter nth,RandomAccessIter last){
    nth_element__(first,nth,last,value_type(first));
}

template<class RandomAccessIter,class T,class Compare>
inline void nth_element__(RandomAccessIter first,RandomAccessIter nth,RandomAccessIter last,T*,Compare comp){
  RandomAccessIter cut;
  while(last -first > 3){
    cut = unguarded_partition(first,last,T(median(*first,*(first+(last-first)/2),*(last-1),comp)));
    if(cut <= nth)
      first = cut;
    else 
      last = cut;
  }
  insert_sort(first,last,comp);
}
//自带比较器版本
template<class RandomAccessIter,class Compare>
inline void nth_element(RandomAccessIter first,RandomAccessIter nth,RandomAccessIter last,Compare comp){
    nth_element__(first,nth,last,value_type(first),comp);
}

template<class InputIterator,class RandomAccessIter,class Distance,class T>
inline RandomAccessIter partial_sort_copy(InputIterator first,InputIterator last,RandomAccessIter result_first,RandomAccessIter result_last,Distance*,T*){
    ///现将范围A内的元素拷贝到B中，再在B中完成堆序化
    auto iter = result_first;
    while(first != last && iter != result_last){
      *iter = *first;
      ++iter;
      ++first;
    }
    //建成堆
    make_heap(first,result_last);

    //范围B偏小，需要对元素进行筛选，保证范围B中的元素是前x个最小的
    while(first != last){
      if(*first < *result_first){
        *result_first = *first;
         adjust_heap(result_first,iter);
      }
    }
    sort_heap(result_first,iter);
    return iter;
}

//将范围A内元素堆序化，排序后输出到B中 --> 不会改变A
//有可能范围B较小,设为x,只能将最小的x个元素拷贝到B中
template<class InputIterator,class RandomAccessIter>
inline RandomAccessIter partial_sort_copy(InputIterator first,InputIterator last,RandomAccessIter result_first,RandomAccessIter result_last){
    
  return partial_sort_copy__(first,last,result_first,result_last,distance_type(first),value_type(first));
}

template<class InputIterator,class RandomAccessIter,class Distance,class T,class Compare>
inline RandomAccessIter partial_sort_copy(InputIterator first,InputIterator last,RandomAccessIter result_first,RandomAccessIter result_last,Distance*,T*,Compare comp){
    ///现将范围A内的元素拷贝到B中，再在B中完成堆序化
    auto iter = result_first;
    while(first != last && iter != result_last){
      *iter = *first;
      ++iter;
      ++first;
    }
    //建成堆
    make_heap(first,result_last,comp);

    //范围B偏小，需要对元素进行筛选，保证范围B中的元素是前x个最小的
    while(first != last){
      if(*first < *result_first){
        *result_first = *first;
        adjust_heap(result_first,iter,comp);
      }
    }
    sort_heap(result_first,iter,comp);
    return iter;
}

//自带比较器版本
template<class InputIterator,class RandomAccessIter,class Compare>
inline RandomAccessIter partial_sort_copy(InputIterator first,InputIterator last,RandomAccessIter result_first,RandomAccessIter result_last,Compare comp){
    
  return partial_sort_copy__(first,last,result_first,result_last,distance_type(first),value_type(first),comp);
}


template<class ForwardIterator,class StrictWeakOrdering>
bool is_sorted(ForwardIterator first,ForwardIterator last,StrictWeakOrdering comp){
  if(first == last)
    return true;
  auto next = first;
  for(;next != last;first = next,++next)
    if(comp(*next,*first))
      return false;
  return true;
}

template<class RandomAccessIter,class Distance>
inline bool is_heap_impl(RandomAccessIter first,RandomAccessIter last,Distance){
  Distance n = last - first;
  Distance parent = 0,child = 1;
  for(;child<n;++child){       //比较两个孩子
    if(first[parent] < first[child])
      return false;
    if((child & 1) == 0)   //右孩子，进入下一层
      ++parent;
  }
  return true;
}

//判断序列是否是堆序化的 ---> 大根堆
template<class RandomAccessIter>
inline bool is_heap(RandomAccessIter first,RandomAccessIter last){
  //需要定义距离类型临时变量
  return is_heap_impl(first,last,distance_type(first));
}

//自带比较器版本
template<class RandomAccessIter,class Distance,class Compare>
inline bool is_heap_impl(RandomAccessIter first,RandomAccessIter last,Distance,Compare comp){
  Distance n = last - first;
  Distance parent = 0,child = 1;
  for(;child<n;++child){       //比较两个孩子
    if(comp(first[parent],first[child]))
      return false;
    if((child & 1) == 0)   //右孩子，进入下一层
      ++parent;
  }
  return true;
}

//判断序列是否是堆序化的 ---> 大根堆
template<class RandomAccessIter,class Compare>
inline bool is_heap(RandomAccessIter first,RandomAccessIter last,Compare comp){
  //需要定义距离类型临时变量
  return is_heap_impl(first,last,distance_type(first),comp);
}


//范围B里的某个元素在范围A内首次出现的位置
template<class InputIterator,class ForwardIterator>
InputIterator find_first_of(InputIterator first1,InputIterator last1,ForwardIterator first2,ForwardIterator last2){
  for(;first1 != last1;++first1)
    for(auto iter = first2; iter != last2;++iter)
      if(*first1 == *iter)
        return first1;
  return last1;
}

template<class InputIterator,class ForwardIterator,class Compare>
InputIterator find_first_of(InputIterator first1,InputIterator last1,ForwardIterator first2,ForwardIterator last2,Compare comp){
  for(;first1 != last1;++first1)
    for(auto iter = first2; iter != last2;++iter)
      if(comp(first1,*iter))
        return first1;
  return last1;
}


//寻找当前序列的下一个排列组合 
//abcd --> acbd
//1423 --> 1432  
//以数字为例:寻找整体大小增长幅度最下的下一个数字
//从右到左寻找第一对相邻的元素xy,满足x<y --> 满足增序排列的第一对 ---> 只要在此基础之上增加最小幅度即可 --> 找到要修改的起点
//后面的元素对都满足x>y ---> 降序排列
//从右向左找到第一个大于>x的元素z,用z顶替x是变化幅度最小的
//交换z与x ---> y后面的元素依旧降序排列 ---> 将其逆转 --> 值最小 --> 配合z,增长幅度就最小
template<class BidirectionalIterator> 
bool next_permutation(BidirectionalIterator first,BidirectionalIterator last){
    //无元素
    if(first == last)
      return false;
    BidirectionalIterator i = first;
    //只有一个元素
    if(++i == last)
      return false;
    i = last;
    --i;
    BidirectionalIterator next_i,n;
    for(;;){
      //next_i和i相邻
      next_i = i;
      --i;
      if(*i < *next_i){
         n = last;
         while(!(*i < *--n));
         iter_swap(i,n);
         reverse(next_i,last);
         return true;
      }
      //dcba ----> abcd
      if(i == first){
        reverse(first,last);
        return false;
      }
    }
}

template<class BidirectionalIterator,class Compare> 
bool next_permutation(BidirectionalIterator first,BidirectionalIterator last,Compare comp){
    //无元素
    if(first == last)
      return false;
    BidirectionalIterator i = first;
    //只有一个元素
    if(++i == last)
      return false;
    i = last;
    --i;
    BidirectionalIterator next_i,n;
    for(;;){
      next_i = i;
      --i;
      if(comp(*i,*next_i)){
         n = last;
         //从右向左找到第一个<*i的元素
         while(!(*--n < *i));
         iter_swap(i,n);
         reverse(next_i,last);
         return true;
      }
      //dcba ----> abcd
      if(i == first){
        reverse(first,last);
        return false;
      }
    }
}

//减小幅度最小的
template<class BidirectionalIterator>
bool prev_permutation(BidirectionalIterator first,BidirectionalIterator last){
      if(first == last)
        return false;
      auto i = first;
      if(++first == last)
        return false;
      i = last;
      --i;
      BidirectionalIterator next_i,n;
      for(;;){
        next_i = i;
        i--;
        if(*next_i < *i){
          n = last;
          while(!(*--n < *i));
          iter_swap(n,i);
          reverse(n,last);
          return true;
        }
        if(i == first){
          reverse(first,last);
          return false;
        }
      }
}

template<class BidirectionalIterator,class Compare>
bool prev_permutation(BidirectionalIterator first,BidirectionalIterator last,Compare comp){
      if(first == last)
        return false;
      auto i = first;
      if(++first == last)
        return false;
      i = last;
      --i;
      BidirectionalIterator next_i,n;
      for(;;){
        next_i = i;
        i--;
        if(comp(*next_i,*i)){
          n = last;
          while(comp(*--n,*i));
          iter_swap(n,i);
          reverse(n,last);
          return true;
        }
        if(i == first){
          reverse(first,last);
          return false;
        }
      }
}

template<class ForwardIterator>
ForwardIterator max_element(ForwardIterator first,ForwardIterator last){
  if(first == last)
    return;
  ForwardIterator max = first;
  while(++first != last)
    if(*max < *first)
      max = first;
  return max;
}
template<class ForwardIterator,class Compare>
ForwardIterator max_element(ForwardIterator first,ForwardIterator last,Compare comp){
  if(first == last)
    return;
  ForwardIterator max = first;
  while(++first != last)
    if(comp(*max,*first))
      max = first;
  return max;
}
template<class ForwardIterator>
ForwardIterator min_element(ForwardIterator first,ForwardIterator last){
  if(first == last)
    return;
  ForwardIterator min = first;
  while(++first != last)
    if(*first < *min)
      min = first;
  return min;
}
template<class ForwardIterator,class Compare>
ForwardIterator min_element(ForwardIterator first,ForwardIterator last,Compare comp){
  if(first == last)
    return;
  ForwardIterator min = first;
  while(++first != last)
    if(comp(*min,*first))
      min = first;
  return min;
}



//set的相关算法
//求排序集合的并集
//123 + 234 ---> 1234
template<class InputIterator1,class InputIterator2,class OutputIterator>
OutputIterator set_union(InputIterator1 first1,InputIterator1 last1,InputIterator2 first2,InputIterator2 last2,OutputIterator result){
  while(first1 != last1 && first2 != last2){
    if(*first1 < *first2){
      *result = *first1;
      ++first1;
    }else if(*first2 < *first1){
      *result = *first2;
      ++first2;
    }else{ ///=
      *result = *first1;
      ++first1;
      ++first2;
    }
    ++result;
  }
}
template<class InputIterator1,class InputIterator2,class OutputIterator,class Compare>
OutputIterator set_union(InputIterator1 first1,InputIterator1 last1,InputIterator2 first2,InputIterator2 last2,OutputIterator result,Compare comp){
  while(first1 != last1 && first2 != last2){
    if(comp(*first1,*first2)){
      *result = *first1;
      ++first1;
    }else if(comp(*first2 ,*first1)){
      *result = *first2;
      ++first2;
    }else{ ///=
      *result = *first1;
      ++first1;
      ++first2;
    }
    ++result;
  }
}


//交集
template<class InputIterator1,class InputIterator2,class OutputIterator>
OutputIterator set_intersection(InputIterator1 first1,InputIterator1 last1,InputIterator2 first2,InputIterator2 last2,OutputIterator result){
  while(first1 != last1 && first2 != last2){
    if(*first1 < *first2){
      ++first1;
    }else if(*first2 < *first1){
      ++first2;
    }else{ /// =时才取出
      *result = *first1;
      ++first1;
      ++first2;
    }
    ++result;
  }
}
template<class InputIterator1,class InputIterator2,class OutputIterator,class Compare>
OutputIterator set_intersection(InputIterator1 first1,InputIterator1 last1,InputIterator2 first2,InputIterator2 last2,OutputIterator result,Compare comp){
  while(first1 != last1 && first2 != last2){
    if(comp(*first1,*first2)){
      ++first1;
    }else if(comp(*first2,*first1)){
      ++first2;
    }else{ /// =时才取出
      *result = *first1;
      ++first1;
      ++first2;
    }
    ++result;
  }
}

//差集 S1-S2 --->只出现在S1中的元素
template <class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator set_difference(InputIterator1 first1, InputIterator1 last1,
                              InputIterator2 first2, InputIterator2 last2,
                              OutputIterator result) {
  while (first1 != last1 && first2 != last2)
    if (*first1 < *first2) { //只接受出现在S1中的元素
      *result = *first1;
      ++first1;
      ++result;
    }
    else if (*first2 < *first1)
      ++first2;
    else {
      ++first1;
      ++first2;
    }
  return copy(first1, last1, result);
}
template <class InputIterator1, class InputIterator2, class OutputIterator, 
          class Compare>
OutputIterator set_difference(InputIterator1 first1, InputIterator1 last1,
                              InputIterator2 first2, InputIterator2 last2, 
                              OutputIterator result, Compare comp) {
  while (first1 != last1 && first2 != last2)
    if (comp(*first1, *first2)) {
      *result = *first1;
      ++first1;
      ++result;
    }
    else if (comp(*first2, *first1))
      ++first2;
    else {
      ++first1;
      ++first2;
    }
  return copy(first1, last1, result);
}

//对称差集 s1-s2 + s2-s1 --> 共同元素被剔除

template <class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator set_symmetric_difference(InputIterator1 first1,
                                        InputIterator1 last1,
                                        InputIterator2 first2,
                                        InputIterator2 last2,
                                        OutputIterator result) {
  while (first1 != last1 && first2 != last2)
    if (*first1 < *first2) {
      *result = *first1;
      ++first1;
      ++result;
    }
    else if (*first2 < *first1) {
      *result = *first2;
      ++first2;
      ++result;
    }
    else {         ///共同元素被剔除
      ++first1;
      ++first2;
    }
  return copy(first2, last2, copy(first1, last1, result));
}
template <class InputIterator1, class InputIterator2, class OutputIterator,
          class Compare>
OutputIterator set_symmetric_difference(InputIterator1 first1,
                                        InputIterator1 last1,
                                        InputIterator2 first2,
                                        InputIterator2 last2,
                                        OutputIterator result, Compare comp) {
  while (first1 != last1 && first2 != last2)
    if (comp(*first1, *first2)) {
      *result = *first1;
      ++first1;
      ++result;
    }
    else if (comp(*first2, *first1)) {
      *result = *first2;
      ++first2;
      ++result;
    }
    else {
      ++first1;
      ++first2;
    }
  return copy(first2, last2, copy(first1, last1, result));
}
}
#endif
