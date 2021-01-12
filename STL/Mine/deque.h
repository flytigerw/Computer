#ifndef DEQUE
#define DEQUE 


#include "alloc.h"
#include "construct.h"
#include "iterator.h"
#include "uninitialized.h"
#include "algorithm.h"

namespace stl{
  
  //获取缓冲大小 
  //n为用户自定义大小
  //若n不为0,返回n
  //若n为0,vaule_size为存储的元素的大小,若value_size<512,,则返回 512/value_size，否则就返回1
  inline size_t deque_buf_size(size_t n,size_t value_size){
      return n!=0 ? n : ((value_size < 512) ? size_t(512/value_size) : size_t(1));
  }

  
  template<class T,class Ref,class Ptr,size_t BufSize>
    struct deque_iterator{
      
      //类型定义
      typedef deque_iterator<T,T&,T*,BufSize> iterator;
      typedef deque_iterator<T,const T&,const T*,BufSize> const_iterator;
      typedef iterator self_type;
      static size_t buffer_size(){
        return deque_buf_size(BufSize,sizeof(T));
      }

      typedef random_access_iterator_tag iterator_category;
      typedef  T value_type;
      typedef  Ptr pointer;
      typedef  Ref reference;
      typedef  size_t size_type;
      typedef  ptrdiff_t difference_type;
      typedef  T** buffer_pointer;   
      typedef  T** buffer_array_pointer;   //缓冲区数组,数组中的米格元素为T*
      
      T* cur;            //指向当前元素
      T* last;          //当前元素所在缓冲区的尾部
      T* first;
      buffer_pointer buf_ptr;   //当前缓冲区位于缓冲区数组的哪一个? ---> 迭代器的移动可能会跨区
      
      deque_iterator(T* x,buffer_pointer y):cur(x),first(*y),last(*y+buffer_size()),buf_ptr(y){}
      
      reference operator*() const{return *cur;}
      pointer  operator->() const{return &(operator*());}
      

      //迭代器的相减
      difference_type operator-(const self_type& x){
          //可能跨区--> 一个区的大小:buffer_size()
          //区的个数---> buf_ptr之差
          return difference_type(buffer_size())*(buf_ptr-x.buf_ptr-1) + (cur-first) + (x.last-x.cur);
      }

      self_type& operator++(){
        //考虑跨区
        ++cur;
        if(cur == last){
          set_buf_ptr(buf_ptr+1);
          cur = first;
        }
        return *this;
      }

      self_type operator++(int){
        auto tmp = *this;
        operator++();
        return tmp;
      }
      self_type& operator--(){
        //考虑跨区
        if(cur == first){
          set_buf_ptr(buf_ptr-1);
          cur = first;
        }
        --cur;
        return *this;
      }

      self_type operator--(int){
        auto tmp = *this;
        operator--();
        return tmp;
      }

      self_type& operator+=(difference_type n){
        //总的偏移量
        difference_type offset = n + (cur-first);
        if(offset >=0 && offset< difference_type(buffer_size())) 
          return cur+=n;
        else{  //考虑跨区
          //offset<0时，肯定会跨越当前区
          difference_type buf_offset = offset>0 ? offset/difference_type(buffer_size()) : -difference_type((-offset-1)/buffer_size() -1 );
          set_buf_ptr(buf_ptr,buf_offset);
          cur = first+(offset-buf_offset*difference_type(buffer_size()));
        }
        return *this;
      }
      self_type& operator -=(difference_type n) {
        return *this+= (-n);
      }
      self_type operator+(difference_type n) const{
        self_type  tmp=*this;
        return tmp +=n;
      }
      self_type operator-(difference_type n) const{
        self_type  tmp=*this;
        return tmp -=n;
      }
      
      reference operator[](difference_type n) const{
        return *(*this+n);
      }

      bool operator==(const self_type& c) const{
        return cur == c.cur;
      }
      bool operator!=(const self_type& c) const{
        return cur != c.cur;
      }

      //缓冲区比较优先
      bool operator<(const self_type& c) const{
        return (buf_ptr == c.buf_ptr) ? (cur < c.cur) : (buf_ptr < c.buf_ptr);
      }
      
      void set_buf_ptr(buffer_pointer new_buf_ptr){
          buf_ptr = new_buf_ptr;
          first = *new_buf_ptr;
          last = first+difference_type(buffer_size());
      }
    };
  
  template<class T,class Alloc=alloc,size_t BufSize=0>
    class deque{
      //容器相关的类型定义
      public:
        typedef  T  value_type;
        typedef  value_type* pointer;
        typedef  const value_type*  const_pointer;
        typedef  value_type& reference;
        typedef  const value_type& const_reference;
        typedef  size_t size_type;
        typedef  ptrdiff_t difference_type;
      
        //迭代器
      public:
        typedef deque_iterator<value_type,reference,pointer,BufSize> iterator;
        typedef deque_iterator<value_type,const_reference,const_pointer,BufSize> const_iterator;
        typedef reverse_iterator<const_iterator> const_reverse_iterator;
        typedef reverse_iterator<iterator> reverse_iterator;

        //Internel typedefs
      protected:
        typedef pointer* buffer_pointer;
        typedef pointer* buffer_array_pointer;
        typedef simple_alloc<value_type,Alloc> data_allocator;
        typedef simple_alloc<pointer,Alloc> buffer_array_allocator;
        static size_type buffer_size(){
          return deque_buf_size(BufSize,sizeof(value_type));
        }
        static size_type initial_buffer_array_size(){
          return 8;
        }


        //数据成员
      protected:
        iterator start;    //指向第一块缓冲区的第一个元素
        iterator finish;   //指向最后一块缓冲区的最后一个元素
        buffer_array_pointer buffer_array; //缓冲区类型value_size*  ---> 构成的数组 --> value_type* *
        size_type buffer_array_size;  //缓冲区个数 --> 可能不够

        //迭代器接口
      public:
        iterator begin(){return start;}
        const_iterator cbegin() const {return start;}
        iterator end(){return finish;}
        const_iterator cend() const {return finish;}
        reverse_iterator rbegin(){return reverse_iterator(finish);}
        reverse_iterator rend(){return reverse_iterator(start);}
        const_reverse_iterator rcbegin() const{return reverse_iterator(finish);}
        const_reverse_iterator rcend()const{return reverse_iterator(start);}

        //其他接口
      public:
        reference operator[](size_type n){
          return start[difference_type(n)];
        }
        const_reference operator[](size_type n)const{
          return start[difference_type(n)];
        }

        reference front() {return *start;}
        //前一个元素
        reference back() {
          iterator tmp = finish;
          --tmp;
          return *tmp;
        }
        const_reference front() const{return *start;}
        const_reference back() const{
          iterator tmp = finish;
          --tmp;
          return *tmp;
        }
          
        //迭代器的-考虑了跨区，这里直接使用
        size_type size() const{
          return finish-start;
        }
        size_type map_size() const{
          return size_type(-1);
        }

        bool empty() const{
          return finish == start;
        }
      
        //ctor and dctor
      public:
        deque() : start(),finish(),buffer_array(nullptr),buffer_array_size(0){
          create_buffer_array_and_buffers(0);
        }
        deque(const deque& x):start(),finish(),buffer_array(nullptr),buffer_array_size(0){
          
          create_buffer_array_and_buffers(x.size());
          //元素拷贝
          try{
            uninitialized_copy(x.begin(),x.end(),start);
          }catch(...){
            throw ;
          }
        }
        //用n个元素初始化
        deque(size_type n,const value_type& value):start(nullptr),finish(nullptr),buffer_array(nullptr),buffer_array_size(0){
          fill_initialize(n,value);
        }
        deque(size_type n):start(nullptr),finish(nullptr),buffer_array(nullptr),buffer_array_size(0){
          fill_initialize(n,value_type());
        }

        //用迭代器范围初始化
        template<class InputIterator>
          deque(InputIterator first,InputIterator last):start(nullptr),finish(nullptr),buffer_array(nullptr),buffer_array_size(0){
            range_initialize(first,last,iterator_category(first));
          }
        
        ~deque(){
          //销毁元素，释放内存
          destory(start,finish);
          dellocate_buffer_array_and_buffers();
        }

        deque& operator=(const deque& x){
              //要比较二者的数量
              const size_type len = size();
              if(this != &this){
                if(len >= x.size()) //拷贝一部分，删除一部分
                  erase(copy(x.begin(),x.end(),start),finish);
                else{
                  //考虑到空间不足,不能完全拷贝，一部分需要插入
                  auto pos = x.begin() + difference_type(len);
                  copy(x.begin(),pos,start);
                  insert(finish,pos,x.end());
                }
              }
              return *this;
        }
        //交换各种指针即可
        void swap(deque& x){
            std::swap(start,x.start);
            std::swap(finish,x.finish);
            std::swap(buffer_array,x.buffer_array);
            std::swap(buffer_array_size,x.buffer_array_size);
        }
        //私有辅助函数
      protected:
        void create_buffer_array_and_buffers(size_type);
        void allocate_buffer(){
          return data_allocator::allocate(buffer_size());
        }

        //从某个元素开始,释放接下来一整个缓冲区
        void deallocate_buffer(pointer p){
          data_allocator::deallocate(p,buffer_size());
        }

        void fill_initialize(size_type n,const value_type&);

        //释放缓冲区数组和各个缓冲区
        void dellocate_buffer_array_and_buffers(){
          //先释放缓冲区
          for(buffer_pointer cur=start.buf_ptr;cur<=finish.buf_ptr;++cur)
            deallocate_buffer(cur);
          //再释放缓冲区数组
          buffer_array_allocator::deallocate(buffer_array,buffer_array_size);
        }

        template<class InputIterator>
          void range_initialize(InputIterator first,InputIterator last,input_iterator_tag){
              create_buffer_array_and_buffers(0);
              for(;first != last; ++first)
                push_back(*first);
          }
        //Forward可以计算迭代器距离---> 批量操作更快
        template<class ForwardIterator>
          void range_initialize(ForwardIterator first,ForwardIterator last,forward_iterator_tag){
              size_type n= 0;
              distance(first,last,n);
              create_buffer_array_and_buffers(n);
              try{
                //copy不成功会会将其销毁
                uninitialized_copy(first,last,n);
              }catch(...){
                //只需释放内存即可
                dellocate_buffer_array_and_buffers();
                throw ;
              }
          }
       
        //重新分配缓冲区
      void reallocate_buffer(size_type buffers_to_add,bool front);

      //在尾部增加新的buffer_ptr
      void add_buffer_at_back(size_t buffers_to_add = 1){
        //超过尾部余量就真的扩容
        if(buffers_to_add+1 > buffer_array_size-(finish.buf_ptr-buffer_array))
          reallocate_buffer(buffers_to_add,false);
      }
      void add_buffer_at_front(size_t buffers_to_add = 1){
        //超过头部余量就真的扩容
        if(buffers_to_add+1 > start.buf_ptr-buffer_array)
          reallocate_buffer(buffers_to_add,true);
      }

      //需要在后面扩容
      void push_back_impl(const value_type& v);
      void push_front_impl(const value_type& v);

      //跨区考虑
      void pop_back_impl();
      void pop_front_impl();

      iterator insert_impl(iterator pos,const value_type& );
      iterator insert_impl(iterator pos,size_type n,const value_type& );
      template<class InputIterator>
        void insert_impl(iterator pos,InputIterator first,InputIterator last,input_iterator_tag);
      template<class ForwardIterator>
        void insert_impl(iterator pos,ForwardIterator first,ForwardIterator last,forward_iterator_tag);

      template<class ForwardIterator>
        void insert_impl(iterator pos,ForwardIterator first,ForwardIterator last,size_type n);
        //插入系列
        //插入系列
        //插入系列
      public:
        void push_back(const value_type& v){
          //空间足够
          if(finish.cur != finish.last-1){
            construct(finish.cur,v);
            ++finish.cur;
          }else 
            push_back_impl(v);
        }
        //类似push_back
        void push_front(const value_type& v){
          if(start.cur != start.first){
            construct(start.cur-1,v);
            --start.cur;
          }else 
            push_front_impl(v);
        }

        void pop_back(){
          //没有跨区
          if(finish.cur != finish.first){
            --finish.cur;
            destory(finish.cur);
          }else 
            pop_back_impl();
        }
        void pop_front(){
          //没有跨区
          if(start.cur != start.last-1){
            destory(start.cur);
            ++start.cur;
          }else 
            pop_front_impl();
        }

        //插入系列
      public:
        //在某一位置前面插入新元素
        //返回指向新插入元素的迭代器
        iterator insert(iterator pos,const value_type& x){
          if(pos.cur == start.cur){
            push_front(x);
            return start;
          }else if(pos.cur == finish.cur){
            push_back(x);
            iterator tmp = finish;
            --tmp;
            return tmp;
          }else 
            return insert_impl(pos,x);
        }
        iterator insert(iterator pos){
          return insert(pos,value_type());
        }
        iterator insert(iterator pos,size_type n,const value_type& x);
        template<class InputIterator>
          void insert(iterator pos,InputIterator first,InputIterator last){
            return insert_impl(pos,first,last,iterator_category(first));
          }

      protected:   
        //检验前面的剩余空间是否足够容纳n个元素
        //若能，则返回距离start.cur前n个的iterator 
        //若不能，则需要调整缓冲区
        //反正要保证前面有足够的空间
        iterator reserve_elements_at_front(size_type n){
          //前面的剩余空间
          size_type front_space = start.cur-start.first;
          if(n>front_space)
            create_space_at_front(n);
          return start-difference_type(n-front_space);
        }
        void create_space_at_front(size_type n){
          size_type new_buffer_num = (n+buffer_size()-1)/buffer_size();
          add_buffer_at_front(new_buffer_num); //只分配了缓冲区指针 ---> 接下来才是要分配缓冲区
          size_type i;
          try{
              for(i=1;i<=new_buffer_num;++i)
                *(start.buf_ptr - i) = allocate_buffer();
          }catch(...){
            for(size_type k=1;k<i;++k)
              deallocate_buffer(*(start.buf_ptr-k));
            throw ;
          }
        }
        iterator reserve_elements_at_back(size_type n){
          //后面的剩余空间
          size_type back_space = finish.last-finish.cur-1;
          if(n>back_space)
            create_space_at_back(n-back_space);
          return finish.cur+difference_type(n);
        }
        void create_space_at_back(size_type n){
          size_type new_buffer_num = (n+buffer_size()-1)/buffer_size();
          add_buffer_at_front(new_buffer_num); //只分配了缓冲区指针 ---> 接下来才是要分配缓冲区
          size_type i;
          try{
              for(i=1;i<=new_buffer_num;++i)
                *(finish.buf_ptr - i) = allocate_buffer();
          }catch(...){
            for(size_type k=1;k<i;++k)
              deallocate_buffer(*(finish.buf_ptr-k));
            throw ;
          }
        }

        //销毁尾部之后缓冲区 ---> 用于异常处理中
        void destory_buffer_at_back(iterator after_finish){
          for(buffer_pointer n=after_finish.buf_ptr;n>finish.buf_ptr;--n)
            deallocate_buffer(*n);
        }
        void destory_buffer_at_front(iterator before_start){
          for(buffer_pointer n=before_start.buf_ptr;n<start.buf_ptr;++n)
            deallocate_buffer(*n);
        }

        //删除系列
        //返回之后的迭代器
        iterator erase(iterator pos){
            auto tmp = pos;
            ++tmp;
            difference_type distance = pos-start;
            //位于前半部分
            if(distance < (size()>>1)){
              copy_backward(start,pos,tmp);
              pop_front();
            }else{
              copy(tmp,finish,pos);
              pop_back();
            }
            return start+distance;
        }
        iterator erase(iterator first,iterator last);

        void clear(){
          //先销毁缓冲区的内容和释放缓冲区-->除开start和finish两端
          for(buffer_pointer b = start.buf_ptr+1;b<finish.buf_ptr;++b){
            destory(*b,*b+buffer_size());
            data_allocator::deallocate(*b,buffer_size());
          } 
          if(start.buf_ptr != finish.buf_ptr){
            destory(start.cur,start.last);
            destory(finish.first,finish.cur);
            data_allocator::deallocate(finish.first,buffer_size());
          }else 
            destory(start.cur,finish.cur);
        finish = start;
        }

        bool operator==(const deque<T,Alloc,0>&x) const{
          return size() == x.size() && is_subset(begin(),end(),x.begin());
        }
        bool operator!=(const deque<T,Alloc,0>&x) const{
          return !operator==(x);
        }
    };




template <class T, class Alloc, size_t BufSize>
  typename deque<T,Alloc,BufSize>::iterator 
  deque<T, Alloc, BufSize>::erase(iterator first,iterator last)  {

    //删除全部
    if(first == start && last == finish){
      clear();
      return finish;
    }
    else{
      difference_type distance  = first-start;
      difference_type  n = last - first;
      //位于前半部分采用copy_backward
      if(distance < (size() - n)/2 ){
        copy_backward(start,first,last);
        iterator new_start = start + n;
        destory(start, new_start);
        //释放对应的缓冲区
        for(buffer_pointer b = start.node;b<new_start.buf_ptr;++b)
          data_allocator::deallocate(*b,buffer_size());
        start = new_start;
      }else{
        copy(last,finish,first);
        iterator new_finish = finish-n;
        destory(new_finish,finish);
        for (buffer_pointer b = new_finish.buf_ptr + 1; b <= finish.buf_ptr; ++b)
        data_allocator::deallocate(*b, buffer_size());
      finish = new_finish;
      }
      return start + distance;
    }
  }
template <class T, class Alloc, size_t BufSize>
  typename deque<T,Alloc,BufSize>::iterator 
  deque<T, Alloc, BufSize>::insert(iterator pos,size_type n,const value_type& v)  {
    iterator border;
    if(pos.cur == start.cur){
        border=reserve_elements_at_front(n);
      uninitialized_fill(border,start,v);
      start = border;
    }
    else if (pos.cur == finish.cur){
      border = reserve_elements_at_back(n);
      uninitialized_fill(finish,border,v);
      finish = border;
    }
    //前两中特殊情况特殊处理，更快
    else insert_impl(pos,n,v);
    }

template <class T, class Alloc, size_t BufSize>
  typename deque<T,Alloc,BufSize>::iterator 
  deque<T, Alloc, BufSize>::insert_impl(iterator pos,size_type n,const value_type& v)  {
      difference_type distance = pos-start;
      if(distance < size()/2){
        iterator border = reserve_elements_at_front(n);
        auto old_start = start;
        pos = start + distance;
        try{
          //需要分两段拷贝
          //前面新开辟的空间采用uninitialized_copy 
          //后面在使用的空间采用copy
          //最后在pos前填充n个元素
          if(distance >= difference_type(n)){ 
              iterator start_n = start + difference_type(n);
              uninitialized_copy(start,start_n,border);
              start = border;
              copy(start_n,n,start);
              fill(pos-difference_type(n),pos,v);
          }else{
              uninitialized_fill_copy(start,pos,border,start,v);
              start = border;
              fill(old_start,pos,v);
          }
        }catch(...){
          destory_buffer_at_front(border);
          throw ;
        }
      }else{
        iterator border = reserve_elements_at_back(n);
        iterator old_finish = finish;
        const difference_type space_left = difference_type(size()-distance);
        try{
          if(space_left > difference_type(n)){
            iterator finish_n = finish - difference_type(n);
            uninitialized_copy(finish_n,finish,finish);
            finish = border;
            copy_backward(pos,finish_n,old_finish);
            fill(pos,pos+difference_type(n),v);
          }else{
            uninitialized_fill_copy(finish,pos+difference_type(n),v,pos,finish);
            finish = border;
            fill(pos,old_finish,v);
          }
        }catch(...){
            destory_buffer_at_back(border);
            throw;
        }
      }
    

  }
template <class T, class Alloc, size_t BufSize>
  template<class ForwardIterator>
  void deque<T, Alloc, BufSize>::insert_impl(iterator pos,ForwardIterator first,ForwardIterator last,size_type n)  {
    //也类似在中间插入n个元素
      difference_type distance = pos-start;
      if(distance < size()/2){
        iterator border = reserve_elements_at_front(n);
        auto old_start = start;
        pos = start + distance;
        try{
          if(distance >= difference_type(n)){ 
              iterator start_n = start + difference_type(n);
              uninitialized_copy(start,start_n,border);
              start = border;
              copy(start_n,n,start);
              copy(first,last,pos-difference_type(n));
          }else{
              ForwardIterator mid = first;
              advance(mid, difference_type(n) - distance);
              uninitialized_copy_copy(start,pos,first,mid,border);
              start = border;
              copy(mid,last,old_start);
          }
        }catch(...){
          destory_buffer_at_front(border);
          throw ;
        }
      }else{
        iterator border = reserve_elements_at_back(n);
        iterator old_finish = finish;
        const difference_type space_left = difference_type(size()-distance);
        try{
          if(space_left > difference_type(n)){
            iterator finish_n = finish - difference_type(n);
            uninitialized_copy(finish_n,finish,finish);
            finish = border;
            copy_backward(pos,finish_n,old_finish);
            copy(first, last, pos);
          }else{
            ForwardIterator mid = first;
            advance(mid, distance);
            uninitialized_copy_copy(mid, last, pos, finish, finish);
            finish = distance;
            copy(first, mid, pos);
          }
        }catch(...){
            destory_buffer_at_back(border);
            throw;
        }
      }
  }
template <class T, class Alloc, size_t BufSize>
  template<class InputIterator>
  void deque<T, Alloc, BufSize>::insert_impl(iterator pos,InputIterator first,InputIterator last,input_iterator_tag)  {
      //插入迭代器,对每个元素调用insert
      copy(first, last, inserter(*this, pos));
  }

template <class T, class Alloc, size_t BufSize>
  template<class ForwardIterator>
  void deque<T, Alloc, BufSize>::insert_impl(iterator pos,ForwardIterator first,ForwardIterator last,forward_iterator_tag)  {
    //可以计算距离
    size_type n = 0;
    distance(first,last,n);
    //类似插入n个元素 ---> 有三种情况
    iterator border;
    if(pos.cur == start.cur){
      border = reserve_elements_at_front(n);
      try{
        uninitialized_copy(first,last,border);
        start = border;
      }catch(...){
        destory_buffer_at_front(border);
        throw;
      }
    }else if(pos.cur == finish.cur){
      border = reserve_elements_at_back(n);
      try{
        uninitialized_copy(first,last,finish);
        finish = border;
      }catch(...){
        destory_buffer_at_front(border);
        throw;
      }
    }else 
      insert_impl(pos,first,last,n);
  }

template <class T, class Alloc, size_t BufSize>
  typename deque<T,Alloc,BufSize>::iterator 
  deque<T, Alloc, BufSize>::insert_impl(iterator pos,const value_type& v)  {
      difference_type distance = pos-start;
      //位于前半部分,前半部分整体向前挪一步
      if(distance < size()/2){
        push_front(front());
        iterator front1 = start;
        ++front1;
        iterator front2 = front1;
        ++front2;
        pos = start+distance; 
        iterator pos1 = pos;
        ++pos1;
        copy(front2,pos1,front1);
      }else{
        push_back(back());
        iterator back1 = finish;
        --back1;
        iterator back2 = finish;
        --back2;
        pos = start+index;
        copy_backward(pos,back2,back1);
      }
      *pos = v;
      return pos;
  }

template <class T, class Alloc, size_t BufSize>
  void deque<T, Alloc, BufSize>::pop_front_impl() {
        
          destory(start.cur);
          //释放该buffer
          deallocate_buffer(start.first);
          //调整最后一个buffer的位置
          start.set_buf_ptr(start.buf_ptr+1);
          start.cur = start.first;
  }
template <class T, class Alloc, size_t BufSize>
  void deque<T, Alloc, BufSize>::pop_back_impl() {
          //释放该buffer
          deallocate_buffer(finish.first);
          //调整最后一个buffer的位置
          finish.set_buf_ptr(finish.buf_ptr-1);
          finish.cur = finish.last-1;
          destory(finish.cur);
  }
template <class T, class Alloc, size_t BufSize>
  void deque<T, Alloc, BufSize>::reallocate_buffer(size_type buffers_to_add,bool front) {
      
    size_type old_buffer_num = finish.buf_ptr-start.buf_ptr + 1;
    size_type new_buffer_num = old_buffer_num+buffers_to_add;
    buffer_pointer new_start;
    //只是尾部缓冲区耗尽,前面还有空余缓冲区 --> 将其整体向前移
    if(buffer_array_size > 2*new_buffer_num){
      new_start = buffer_array+(buffer_array_size-new_buffer_num)/2 + (front ? buffers_to_add : 0);
        
      //无重叠区域,直接范围拷贝
      if(new_start < start.buf_ptr)
       copy(start.buf_ptr,finish.buf_ptr+1,new_start);
      else //含有重叠区域，只能从后向前拷贝
        copy_backward(start.buf_ptr,finish.buf_ptr,new_start+old_buffer_num);
    }else{  //确实需要分配新内存进行扩充
      size_type new_buffer_array_size = buffer_array_size + std::max(buffer_array_size + buffers_to_add) + 2;
      
      buffer_array_pointer new_buffer_array = buffer_array_allocator::allocate(new_buffer_array_size);
      
      //依旧先使用中间部分
      new_start = new_buffer_array+(new_buffer_array_size-new_buffer_num)/2 + (front ? buffers_to_add:0);
      //只用拷贝buffer_ptr
      copy(start.buf_ptr,finish.buf_ptr+1,new_start);
      //释放原内存
      buffer_array_allocator::deallocate(buffer_array,buffer_array_size);
      buffer_array = new_buffer_array;
      buffer_array_size = new_buffer_array_size;
    }
    start.set_buf_ptr(new_start);
    finish.set_buf_ptr(new_start+old_buffer_num-1);
  }

template <class T, class Alloc, size_t BufSize>
  void deque<T, Alloc, BufSize>::push_back_impl(const value_type& v) {

    //在尾部增加新的缓冲区指针
    add_buffer_at_back();
    //分配缓冲区
    *(finish.buf_ptr+1) = allocate_buffer();
    try{
      //构造缓冲区
      construct(finish.cur,v);
      finish.set_buf_ptr(finish.buf_ptr+1);
      finish.cur = finish.first;
    }catch(...){
      deallocate_buffer(*(finish.buf_ptr+1));
      throw ;
    }
  }

template <class T, class Alloc, size_t BufSize>
  void deque<T, Alloc, BufSize>::push_front_impl(const value_type& v) {

    //在头部增加新的缓冲区指针
    add_buffer_at_front();
    //分配缓冲区
    *(start.buf_ptr-1) = allocate_buffer();
    try{
      start.set_buf_ptr(start.buf_ptr-1);
      start.cur = start.first;
      //构造缓冲区
      construct(start.cur,v);
    }catch(...){
      start.set_buf_ptr(start.buf_ptr + 1);
      start.cur = start.last-1;
      deallocate_buffer(*(start.buf_ptr-1));
      throw ;
    }
  }
  //传入大概需要容纳多少元素
template <class T, class Alloc, size_t BufSize>
  void deque<T, Alloc, BufSize>::create_buffer_array_and_buffers(size_type elements_num) {
    size_type buffer_num = elements_num / buffer_size() + 1;
  
    buffer_array_size = std::max(initial_buffer_array_size(),buffer_num+2);
    //先分配缓冲区指针数组
    buffer_array = buffer_array_allocator::allocate(buffer_array_size);
    
    //让要先使用的缓冲区集中在缓冲区数组的中央阶段 ---> 稍后就可向两端扩大
    buffer_pointer s = buffer_array+(buffer_array_size - buffer_num) / 2;  //预留xx/2的缓冲区没用
    buffer_pointer f = s + buffer_num - 1;
    //分配缓冲内存
    buffer_pointer cur;
    try{
      for(cur = s;cur<=f;cur++)
        *cur = allocate_buffer();
    }catch(...){
      for(auto i=s;i<cur;i++)
        deallocate_node(*i);
      buffer_array_allocator::deallocate(buffer_array,buffer_array_size);
      throw;
    }
  }

template <class T, class Alloc, size_t BufSize>
  void deque<T, Alloc, BufSize>::fill_initialize(size_type n,const value_type& v) {
    
    create_buffer_array_and_buffers(n);
    //填充每个缓冲区
    buffer_pointer cur;
    try{
      for(cur = start.buf_ptr;cur < finish.buf_ptr;++cur)
        uninitialized_fill(*cur,*cur+buffer_size(),v);
       uninitialized_fill(finish.first,finish.cur,v);
    }catch(...){
      //销毁元素，释放空间
      for(auto tmp=start.buf_ptr;tmp<cur;++n) 
        destory(*tmp,*tmp+buffer_size());
      dellocate_buffer_array_and_buffers();
      throw;
    }
}

}

#endif
