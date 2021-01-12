


#ifndef HEAPALGORITHM
#define HEAPALGORITHM 


namespace stl{

//堆的一些算法 ---> 用动态数组表示堆
//插入新元素后，自底向上地维护堆序性
//first和last分别对应于begin()和end

template<class RandomAccessIterator,class Distance,class T>
inline void 
rebuild_heap_for_push__(RandomAccessIterator first,RandomAccessIterator last,Distance*,T*){

   //需要从根下标为(n-1)/2的子树开始向上调整
   Distance cur = last-first-1;  //最后一个元素的下标
   T value= *(first+cur);
   Distance root_index = 0;    //根节点下标
   Distance parent = (cur-1)/2;
   //大根堆:大者上升,小者下滑
   while(cur > root_index && *(first+parent) < value){
      *(first+cur) = *(first+parent);  //小者下滑 --> 大者就为value
      cur = parent;
      parent = (parent-1) / 2;
   }
   *(first+cur) = value;
}
template<class RandomAccessIterator>
inline void 
rebuild_heap_for_push(RandomAccessIterator first,RandomAccessIterator last){
  //我们需要迭代器之间的距离的类型，以及元素类型 --> 通过模板推到而得
  rebuild_heap_for_push__(first,last,distance_type(first),value_type(first));
}

//带比较器版本
template<class RandomAccessIterator,class Distance,class T,class Compare>
inline void 
rebuild_heap_for_push__(RandomAccessIterator first,RandomAccessIterator last,Distance*,T*,Compare comp){

   //需要从根下标为(n-1)/2的子树开始向上调整
   Distance cur = last-first-1;  //最后一个元素的下标
   T value= *(first+cur);
   Distance root_index = 0;    //根节点下标
   Distance parent = (cur-1)/2;
   //大根堆:大者上升,小者下滑
   while(cur > root_index && comp(*(first+parent),value)){
      *(first+cur) = *(first+parent);  //小者下滑 --> 大者就为value
      cur = parent;
      parent = (parent-1) / 2;
   }
   *(first+cur) = value;
}

template<class RandomAccessIterator,class Compare>
inline void 
rebuild_heap_for_push(RandomAccessIterator first,RandomAccessIterator last,Compare comp){
  rebuild_heap_for_push__(first,last,distance_type(first),value_type(first),comp);
}

template<class RandomAccessIterator,class Distance,class T>
inline void 
rebuild_heap_for_pop_impl(RandomAccessIterator first,Distance root_index,T root_value,Distance len){
    //从根节点开始，自顶向下调整堆序性
    Distance parent= root_index;
    Distance max_child= 2*parent+2;
    while(max_child < len){
      //让max_child存储着child值最大的下标
      if(*(first+max_child) < *(first+max_child-1))
       --max_child;
      //顶替父节点
      *(first+parent) = *(first+max_child);
      //向下层深入
      parent = max_child;
      max_child = 2*(max_child+1);
    }
    //考虑最后的特殊情况:最后子树的右儿子不存在
    if(max_child == len){
      *(first+parent) = *(first+max_child-1);
      parent = max_child-1;
    }
    //将要删除的根节点放到最后
    *(first+parent) = root_value;
}

template<class RandomAccessIterator,class Distance,class T>
inline void 
rebuild_heap_for_pop__(RandomAccessIterator first,RandomAccessIterator last,Distance*,T*){
    rebuild_heap_for_pop_impl(first,Distance(0),*first,Distance(last-first));
}
//first和last同上
template<class RandomAccessIterator>
inline void 
rebuild_heap_for_pop(RandomAccessIterator first,RandomAccessIterator last){
    rebuild_heap_for_pop__(first,last,distance_type(first),value_type(first));
}


//带比较器版本
template<class RandomAccessIterator,class Distance,class T,class Compare>
inline void 
rebuild_heap_for_pop_impl(RandomAccessIterator first,Distance root_index,T root_value,Distance len,Compare comp){
    Distance parent= root_index;
    Distance max_child= 2*parent+2;
    while(max_child < len){
      if(comp(*(first+max_child), *(first+max_child-1)))
       --max_child;
      *(first+parent) = *(first+max_child);
      parent = max_child;
      max_child = 2*(max_child+1);
    }
    if(max_child == len){
      *(first+parent) = *(first+max_child-1);
      parent = max_child-1;
    }
    *(first+parent) = root_value;
}

template<class RandomAccessIterator,class Distance,class T,class Compare>
inline void 
rebuild_heap_for_pop__(RandomAccessIterator first,RandomAccessIterator last,Distance*,T*,Compare comp){
    rebuild_heap_for_pop_impl(first,Distance(0),*first,Distance(last-first),comp);
}
//带有比较器版本
template<class RandomAccessIterator,class Compare>
inline void 
rebuild_heap_for_pop(RandomAccessIterator first,RandomAccessIterator last,Compare comp){
    rebuild_heap_for_pop__(first,last,distance_type(first),value_type(first),comp);
}

template<class RandomAccessIterator,class Distance,class T>
inline void 
adjust_heap_impl(RandomAccessIterator first,Distance root_index,Distance len){
  Distance parent = root_index;
  Distance max_child = 2*parent+2;
  while(max_child < len){
    if(*(first+max_child) < *(first+max_child-1))
      --max_child;
    if(*(first+max_child) < *(first+parent))
      break;
    else{
      iter_swap((first+max_child),(first+parent));
      parent = max_child;
      max_child = 2*parent+2;
    }
  }
}

//自顶向下的调整堆序型
template<class RandomAccessIterator,class Distance,class T>
inline void 
adjust_heap__(RandomAccessIterator first,RandomAccessIterator last,Distance*,T*){
    adjust_heap_impl(first,Distance(0),Distance(last-first));
}

template<class RandomAccessIterator>
inline void 
adjust_heap(RandomAccessIterator first,RandomAccessIterator last){
  adjust_heap__(first,last,distance_type(first),value_type(first));
}

template<class RandomAccessIterator,class Distance,class T,class Compare>
inline void 
adjust_heap_impl(RandomAccessIterator first,Distance root_index,Distance len,Compare comp){
  Distance parent = root_index;
  Distance max_child = 2*parent+2;
  while(max_child < len){
    if(comp(*(first+max_child),*(first+max_child-1)))
      --max_child;
    if(comp(*(first+max_child),*(first+parent)))
      break;
    else{
      iter_swap((first+max_child),(first+parent));
      parent = max_child;
      max_child = 2*parent+2;
    }
  }
}

//自顶向下的调整堆序型
template<class RandomAccessIterator,class Distance,class T,class Compare>
inline void 
adjust_heap__(RandomAccessIterator first,RandomAccessIterator last,Distance*,T*,Compare comp){
    adjust_heap_impl(first,Distance(0),Distance(last-first),comp);
}
//带有比较器版本
template<class RandomAccessIterator,class Compare>
inline void 
adjust_heap(RandomAccessIterator first,RandomAccessIterator last,Compare comp){
  adjust_heap__(first,last,distance_type(first),value_type(first),comp);
}





template<class RandomAccessIter,class T,class Distance>
void make_heap_impl(RandomAccessIter first,RandomAccessIter last,T*,Distance*){
    //元素个数
    Distance len = last-first;
    if(len < 2)
      return;
    Distance cur = (len-2) / 2;
    while(true){
        rebuild_heap_for_pop_impl(first,cur,*(first+cur),len);     
        if(cur == 0)
          return;
        --cur;
    }

}
//将现有数据不断调整为堆 ---> 自底向上不断调整堆序性即可
template <class RandomAccessIterator>
inline void make_heap(RandomAccessIterator first, RandomAccessIterator last) {
 make_heap_impl(first, last, value_type(first), distance_type(first));
}


template<class RandomAccessIter,class T,class Distance,class Compare>
void make_heap_impl(RandomAccessIter first,RandomAccessIter last,T*,Distance*,Compare comp){
    //元素个数
    Distance len = last-first;
    if(len < 2)
      return;
    Distance cur = (len-2) / 2;
    while(true){
        rebuild_heap_for_pop_impl(first,cur,*(first+cur),len,comp);     
        if(cur == 0)
          return;
        --cur;
    }
}


//带有比较器版本
template <class RandomAccessIterator,class Compare >
inline void make_heap(RandomAccessIterator first, RandomAccessIterator last,Compare comp) {
 make_heap_impl(first, last, value_type(first), distance_type(first),comp);
}


template<class RandomAccessIter>
void sort_heap(RandomAccessIter first,RandomAccessIter last){
  while((last-first)>1)
    rebuild_heap_for_pop(first,last--); //-->元素个数减少
}

template<class RandomAccessIter,class Compare>
void sort_heap(RandomAccessIter first,RandomAccessIter last,Compare comp){
  while((last-first)>1)
    rebuild_heap_for_pop(first,last--,comp); //-->元素个数减少
}

}

#endif
