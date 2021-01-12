

#ifndef VECTOR
#define VECTOR 

#include "iterator.h"
#include "alloc.h"
#include "uninitialized.h"
#include "construct.h"
#include <cstddef>


namespace stl{

  template<class T,class Alloc = alloc>
  class vector{
    public:
      //迭代器就是原生指针,最高级别
      typedef T value_type;
      typedef value_type*  iterator;
      typedef const value_type*  const_iterator;
      typedef stl::reverse_iterator<const_iterator> const_reverse_iterator;
      typedef stl::reverse_iterator<iterator> reverse_iterator;
      //用于榨汁机的迭代器相应类型
      typedef value_type*  pointer;
      typedef const value_type*  const_pointer;
      typedef value_type& reference;
      typedef const value_type& const_reference;
      typedef ptrdiff_t difference_type;
      typedef random_access_iterator_tag iterator_category;
      typedef size_t size_type;
      typedef vector<T,Alloc> self_type;
    protected:
      typedef simple_alloc<value_type,Alloc> data_allocator;
      //beg--end_of_storage为整个vector内存区间
      //但只有beg--finish区间被构造过
      iterator start;
      iterator finish;
      iterator end_of_storage;
    //内存操作
    protected:
      //分配n个空间，并用迭代器指向的元素范围构造n个空间
      template<class ForwardIterator>
        iterator 
        allocate_and_copy(size_type n,ForwardIterator first,ForwardIterator last){
          //先分配内存
          iterator dest = data_allocator::allocate(n);
          try{
            uninitialized_copy(first,last,dest);              
            return dest;
          }catch(...){
            data_allocator::deallocate(dest,n);
            throw ;
          }
        }
      //分配n个空间，并用同一元素构造n个空间
      iterator allocate_and_fill(size_type n,const T&x){
          iterator dest = data_allocator::allocate(n);
          try{
            uninitialized_fill_n(dest,n,x);
            return dest;
          }catch(...){
            data_allocator::deallocate(dest,n);
            throw ;
          }
      }
      //辅助函数
      void fill_initialize(size_type n,const T& value){
        start = allocate_and_fill(n,value);
        finish = start+n;
        end_of_storage = finish;
      }
      void deallocate(){
        if(start)
          data_allocator::deallocate(start,end_of_storage-start);
      }
      void insert_impl(iterator position,const T&x);
      template<class InputIterator>
      void range_insert(iterator pos,InputIterator first,InputIterator last);
      //对外接口
    public:
      //迭代器相关
      iterator begin() {return start;}
      const_iterator cbegin() const {return start;}
      iterator end() {return finish;}
      const_iterator cend() const {return finish;}
      reverse_iterator rbegin() { return reverse_iterator(end()); }
      const_reverse_iterator rbegin() const {return reverse_iterator(end());}
      reverse_iterator rend() { return reverse_iterator(begin()); }
      const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
      
      //容量相关
      //元素个数
      size_type size() const{return size_type(finish-start);}
      //当前含有的总容量
      size_type capacity() const{return size_type(end_of_storage-start);}
      //最大容量空间
      size_type max_size() const {return size_type(-1);}
      bool empty() const{return begin() == end();}
      
      //重载[]
      reference operator[](size_type n) const {
        return *(begin()+n);
      }

      //构造函数
      vector():start(0),finish(0),end_of_storage(0){}
      //用n个相同的元素进行初始化
      vector(size_type n,const T& value){
        fill_initialize(n,value);
      }
      vector(long n,const T& value){
        fill_initialize(n,value);
      }
      vector(int n,const T& value){
        fill_initialize(n,value);
      }
      //容器大小初始化为n,填入默认元素
      explicit vector(size_type n){
        fill_initialize(n,T());
      }
      
      //copy ctor 
      vector(const self_type& x){
        //内存分配，元素拷贝,数据修改
        start= allocate_and_copy((x.end()-x.begin()),x.begin(),x.end());
        end_of_storage = finish = start+ (x.end()-x.begin());
      }
      //copy = 
      self_type& operator=(const self_type& x ){
        if(&x != this){
          if(x.size() > capacity()){  //需要扩容
             iterator new_start = allocate_and_copy(x.end()-x.end(),x.begin(),x.end());
             destory(start,finish);
             deallocate();
             start= new_start;
             end_of_storage = start+(x.end()-x.begin());
          }else if(size() >= x.size()){
            //拷贝一部分,剩下的销毁
            iterator i = copy(x.begin(),x.end(),begin());
            destory(i,finish);
          }else {
            //拷贝一部分，构造一部分
            copy(x.begin(),x.begin()+size(),begin());
            uninitialized_copy(x.begin()+size(),x.end(),finish);
          }
          finish = start+ x.size();
        }
      }
      //用一对迭代器进行初始化
      vector(const_iterator first,const_iterator last){
        //内存分配，元素拷贝,数据修改
        size_type n = 0;
        distance(first,last,n);
        start= allocate_and_copy(n,first,last);
        end_of_storage = finish = start+n; 
      }
      
      ~vector(){
        //调用元素的析构函数
        destory(start,finish);
        //归还内存
        deallocate();
      }

      //调整容器容量 ---> 向大调  ---> 需要进行新内存分配与构造，原空间的销毁
      void reserve(size_type n){
        if(capacity() < n){
          const size_type old_size = size();
          iterator tmp = allocate_and_copy(n,start,end_of_storage);
          destory(start,finish);
          deallocate();
          start = tmp;
          finish = start+ old_size;
          end_of_storage = start+ n;
        }
      }

      const_reference front() const {
        return *cbegin();
      }
      const_reference back() const {
        return *(cend()-1);
      }

      //容器的交换 ----> 交换对应的三个指针即可
      void swap(self_type& x){
        std::swap(start,x.beg);
        std::swap(finish,x.finish);
        std::swap(end_of_storage,x.end_of_storage);
      }

      void push_back(const T& x){
        //若空间没满，直接构造最后一个空间
        if(finish != end_of_storage){
          construct(finish,x);
          ++finish;
        }
        else 
          insert_impl(end(),x);
      }
      
      //在指定位置处插入新元素
      iterator insert(iterator pos,const T&x){
        size_type  n = pos-start;
        if(finish != end_of_storage && pos == end()){
          construct(finish,x);
          ++finish;
        }else 
          insert_impl(pos,x);
        return start+n;
      }

      //插入迭代器指向的范围内的元素
      template<class InputIterator>
        void insert(iterator pos,InputIterator first,InputIterator last){
          range_insert(pos,first,last);
        }

      void pop_back(){
        --finish;
        destory(finish);
      }
      //根据迭代器位置删除元素 ---> 向前覆盖 再destory
      iterator erase(iterator pos){
        if(pos+1 != end())   //非最后一个元素
          copy(pos+1,finish,pos);  //向前覆盖
        --finish;
        destory(finish);
        return pos;   
      }
      //删除范围内的元素
      iterator erase(iterator first,iterator last){
        //向前覆盖，再destory
        auto iter =copy(last,finish,first);
        destory(iter,finish);
        finish -= (last-first);
        return first;
      }

      //调整size 
      void resize(size_type new_size,const T& x){
        if(new_size < size()) //元素删除
          erase(begin()+new_size,end());
        else 
          //元素增加
          insert(end(),new_size-size(),x);
      }
      void resize(size_type new_size) { resize(new_size, T()); }
      void clear() { erase(begin(), end()); }
  };
  template<class T,class Alloc>
    inline bool 
    operator == (const vector<T,Alloc>& x,const vector<T,Alloc>& y){
      return x.size() == y.size() && is_subset(x.begin(),y.begin());
    }
  template <class T, class Alloc>
  inline void swap(vector<T, Alloc>& x, vector<T, Alloc>& y) {
    x.swap(y);
  }
  //在某一个位置出插入新元素
  //需要将该位置之后的元素都向后挪一个单位
  template<class T,class Alloc>
  void vector<T,Alloc>::insert_impl(iterator position,const T&x){
    //空间足够
    if(finish != end_of_storage){
      //定点构造最后一个单位的空间
      construct(finish,*(finish-1));
      ++finish;
      //从后向前拷贝
      stl::copy_backward(position,finish-2,finish-1);
      //填入新元素
      *position = x;
    }else{  //空间不够,二倍扩容
      size_type old_size = size();
      //空间分配
      size_type new_len = (old_size == 0) ? 1 : 2*old_size;
      iterator new_start = data_allocator::allocate(new_len);
      iterator new_finish = new_start;
      try{
       //分两段元素拷贝，中间进行"插入"
       new_finish = uninitialized_copy(start,position,new_start);
      construct(new_finish,x);
      ++new_finish;
      new_finish = uninitialized_copy(position,finish,new_finish);
      }catch(...){
        stl::destory(new_start,new_finish);
        data_allocator::deallocate(new_start,new_len);
        throw ;
      }
    //销毁原有容器
    stl::destory(begin(),end());
    deallocate();
    start= new_start;
    finish = new_finish;
    end_of_storage =start+new_len;
    }
  }
  
  template<class T,class Alloc>
   template<class InputIterator>
   void vector<T,Alloc>::range_insert(iterator pos,InputIterator first,InputIterator last){
      size_type n = distance(last,first);
      size_type left_space = end_of_storage-finish;
      if(left_space >= n){   //不用扩容
        size_type elems_after_pos = finish - pos;
        iterator old_finish = finish;
        if(elems_after_pos > n){
          //先对finish后面没有使用的空间进行构造
          uninitialized_copy(finish-n,finish,finish);
          //将pos -- finish-n的元素向后挪动
          stl::copy_backward(pos,old_finish-n,old_finish);
          //再讲范围内的元素拷贝到pos -- finish-n 
          copy(first,last,pos);
        }else{
          auto tmp = first;
          advance(tmp,elems_after_pos);
          uninitialized_copy(tmp,last,finish);
          finish += (n-elems_after_pos);
          uninitialized_copy(pos,old_finish,finish);
          finish += elems_after_pos;
          copy(first,tmp,pos);
        }
      }else{  //需要扩容
         size_type old_size = size();
         size_type new_len = old_size+std::max(old_size,n);
         iterator new_start = data_allocator::allocate(new_len);
         iterator new_finish = new_start + new_len;
         try{
            //分三段拷贝
            new_finish = uninitialized_copy(start,pos,new_start);
            new_finish = uninitialized_copy(first,last,new_finish);
            new_finish = uninitialized_copy(pos,finish,new_finish);
         }catch(...){
           destory(new_start,new_finish);
           data_allocator::deallocate(new_start,new_len);
           throw;
         }
         //销毁原有容器
         destory(start,finish);
         deallocate();
         start= new_start;
         finish = new_finish;
         end_of_storage = new_start + new_len;
      }
   }

}


#endif
