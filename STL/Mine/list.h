

#ifndef LIST 
#define LIST 

#include "alloc.h"
#include "construct.h"
#include "iterator.h"
#include <iostream>
namespace stl{
    
  template<class T>
    struct list_node{
      list_node* prev,*next;
      T data;
    };
  
  template<class T,class Ref,class Ptr>
    struct list_iterator{
      //类型别名，方便一会儿使用
      typedef list_iterator<T,T&,T*> iterator;
      typedef list_iterator<T,const T&,const T*>const_iterator;

      typedef list_iterator<T,Ref,Ptr> self_type;

      //用于迭代器榨汁机的类型定义
      typedef bidirectional_iterator_tag iterator_category;
      typedef T   value_type;
      typedef Ref reference;
      typedef Ptr pointer   ;
      typedef ptrdiff_t difference_type;
      typedef size_t size_type;
      typedef list_node<T>  node;
      
      node* node_ptr;

      list_iterator(node* n): node_ptr(n){}
      list_iterator(){}
      list_iterator(const iterator& x) : node_ptr(x.node_ptr){}

      bool operator==(const self_type& x) const{
        return node_ptr== x.node_ptr;
      }
      bool operator!=(const self_type& x) const{
        return node_ptr!= x.node_ptr;
      }
      reference operator*()const{
        return node_ptr->data;
      }
      pointer operator->()const{
        return &(operator*());
      }
      self_type& operator++(){
        node_ptr = node_ptr->next;
        return *this;
      }
      self_type operator++(int){
        self_type tmp = *this;
        operator++();
        return tmp;
      }
      self_type& operator--(){
        node_ptr = node_ptr->prev;
        return *this;
      }
      self_type operator--(int){
        self_type tmp = *this;
        operator--();
        return tmp;
      }
    };


  template<class T,class Alloc = alloc>
    class list{
      protected:
        typedef list_node<T> node;
        typedef simple_alloc<node,Alloc> node_allocator;
        typedef list<T,Alloc> self_type;
      public:
        typedef T value_type;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        
        //迭代器相关
      public:
        typedef list_iterator<value_type,reference,pointer> iterator;
        typedef list_iterator<value_type,const_reference,const_pointer> const_iterator;
        typedef stl::reverse_iterator<const_iterator> const_reverse_iterator;
        typedef stl::reverse_iterator<iterator> reverse_iterator;
        iterator begin(){return (head->next);}
        const_iterator cbegin()const{return (head->next);}
        iterator end(){return head;}
        const_iterator cend()const{return head;}
        reverse_iterator rbegin(){return reverse_iterator(end());}
        const_reverse_iterator rcbegin()const{return const_reverse_iterator(end());}

      protected:
        node* head;
        //内存相关
      protected:
        node* get_node_memory(){
          return node_allocator::allocate();
        }
        void return_node_memory(node* p){
          node_allocator::deallocate(p);
        }
        node* construct_node(const T& x){
          node* p = get_node_memory();
          try{
            construct(&p->data,x);
          }catch(...){
            return_node_memory(p);
            throw;
          }
          return p;
        }
        void destory_node(node* p){
          desctruct(&p->data);
          return_node_memory(p);
        }

      //辅助函数
      protected:
        void empty_initialize(){
          head = get_node_memory();
          head->next = head->prev = head;
        }
        //构造n个节点,值都为v
        void fill_initialize(size_type n,const T& v){
          empty_initialize();
          try{
            insert(begin(),n,v);
          }catch(...){
            clear();
            return_node_memory(head);
            throw;
          }
        }
        
        template<class InputIterator>
          void range_initialize(InputIterator first,InputIterator last){
            empty_initialize();
            try{
              insert(begin(),first,last);
            }catch(...){
              clear();
              return return_node_memory(head);
              throw;
            }
          }
        //ctors
      public:
        template<class InputIterator>
          list(InputIterator first,InputIterator last){
            range_initialize(first,last);
          }
        
        list(const self_type& x){
          range_initialize(x.begin(),x.end());
        }
        list(){
          empty_initialize();
        }
        self_type& operator=(const self_type& x){
           //不采用clear()+insert(范围)的方式 ---> 两次遍历
           //争取一次遍历就完成任务
           //重点在于:若x的链表更长,则会insert多余节点
           //若x.的链表偏短，则会erase本链表的多余节点
           //确定出多余节点的位置
          if(this != &x){
            iterator f1 = begin();
            iterator l1 = end();
            iterator f2 = x.begin();
            iterator l2 = x.end();
            while(f1 != l1 && f2 != l2){
              *f1++ = *f2++;
            }

            if(l2 == f2)  //x链表偏短
              erase(f1,l1);
            else 
              insert(f1,f2,l2);
          }

        }
        ~list(){
          clear();
          return_node_memory(head);
        }
        //对外接口
      public:
        bool empty() const {return head->next == head;}
        size_type size() const{
          size_type n = 0;
          distance(begin(),end(),n);
          return n;
        }
        size_type max_size() const {return size_type(-1);}
        reference front() {
          return *begin();
        }
        const_reference front()const {
          return *cbegin();
        }

        reference back(){
          return *(--end());
        }
        const_reference back()const{
          return *(--cend());
        }
        
        //交换head即可
        void swap(self_type& x){
            auto tmp = head;
            head = x.head;
            x.head = tmp;
        }

        //插入系列
        //在指定的位置前面插入新的节点
        iterator insert(iterator pos,const T& v){
          node* tmp = construct_node(v);
          tmp->next = pos.node_ptr;
          tmp->prev = pos.node_ptr->prev;
          pos.node_ptr->prev->next = tmp;
          pos.node_ptr->prev = tmp;
          std::cout << "xx" << std::endl;
          return tmp;
        }
        //插入默认元素
        iterator insert(iterator pos){
          return insert(pos,T());
        }
        //插入迭代器范围元素
        template<class InputIterator>
          void insert(iterator pos,InputIterator first,InputIterator last){
           for(;first != last;++first)
             insert(pos,*first);
          }

        //在某一位置前插入n个元素
        void insert(iterator pos,size_type n,const T& x){
          for(;n>0;n--)
            insert(pos,x);
        }
        void push_back(const T& x){
          insert(end(),x);
        }
        void push_front(const T& x){
          insert(begin(),x);
        }

        //向上调整大小,多出来的节点用x构造
        void resize(size_type new_size,const T& x){
          iterator i = begin();
          size_type len = 0;
          //先计算链表大小,将其与new_size比较
          for(;i!=end() && len < new_size; ++i,++len);
          if(len == new_size)  //链表偏大 --> 裁掉尾部
            erase(i,end());
          else 
            insert(end(),new_size-len,x);
        }
        void resize(size_type new_size){
            resize(new_size,T());
        }
        //删除系列
        //返回删除后的下一节点
        iterator erase(iterator pos){
            pos.node_ptr->prev->next = pos.node_ptr->next;
            pos.node_ptr->next->prev = pos.node_ptr->prev;
            auto next_node = pos.node_ptr->next;
            destory_node(pos.node_ptr);
            return next_node;
        }
       //删除范围 [)
        iterator erase(iterator first,iterator last){
            while(first != last)
              erase(first++);
            return last;
        }
        //删除所有链表数据，只剩下head
        void clear(){
          auto cur = head->next;
          node* tmp;
          while(cur != head){
            tmp = cur;
            cur = cur->next;
            destory_node(tmp);
          }
          head->next = head->prev = head;
        }
        void pop_front(){
          erase(begin());
        }
        void pop_back(){
          iterator tmp = end();
          erase(--tmp);    //不能采用--end() ---> end()指向不对
        }


      protected:
        //将链表的某一范围进行位置转移--> pos之前
        //转移范围[)
        void transfer(iterator pos,iterator first,iterator last){
          if(pos != last){
            first.node_ptr->prev->next = last.node_ptr;
            last.node_ptr->prev = pos.node_ptr;

            first.node_ptr->prev = pos.node_ptr->prev;
            pos.node_ptr->prev->next = first.node_ptr;

            pos.node_ptr->prev = last.node_ptr->prev;
            last.node_ptr->prev->next = pos.node_ptr;
          }
        }
        //自带函数
      public:
        //将链表x接到本链表pos之前
        //会改变x --> 采用引用
        void splice(iterator pos,list& x){
          if(!x.empty())
            transfer(pos,x.begin(),x.end());
        }
        void splice(iterator pos,iterator first,iterator last){
          if(first != last)
            transfer(pos,first,last);
        }
        //将x转移到pos之前
        void splice(iterator pos,iterator x){
            iterator tmp = x;
            if(pos == x || pos== ++tmp)
              return;
            transfer(pos,x,tmp);
        }
        
        //删掉链表中所有值为v的节点
        void remove(const T& v){
            iterator first = begin();
            while(first != end()){
              if(*first == v)
                first = erase(first);
              else 
                ++first;
            }
            
        }
        
        //删除链表中的重复节点 ---> 前提是重复节点紧挨着
        void unique(){
            if(empty())
              return;
            iterator first = begin();
            iterator next = first;
            while(++next != end()){
              if(*next == *first)
                erase(next);
              else 
                first = next;
              next = first;
            }
        }
      
        //合并两条有序链表
        void merge(list& x){
          iterator f2 = begin();
          iterator l2 = end();
          iterator f1 = x.begin();
          iterator l1 = x.end();
          iterator tmp;
          while(f1 != l1 && f2 != l2)
            if(*f2 < *f1){
              tmp = f1;
              transfer(f2,f1,++tmp);
              f1 = tmp;
            }else ++f1;
          if(l2 != f2) //链表x过长,则将其合并到尾部
            transfer(f2,f1,l1);
        }
        
        //链表逆序
        void reverse(){
          //0或1个节点时,直接返回
          if(empty() || head->next->next == head) 
            return;
          iterator first = begin();
          ++first;
          iterator old;
          while(first != end()){
              old = first;
              ++first;
              transfer(begin(),old,first);
          }
        }
        //排序
        void sort(){
          //0或1个节点时,直接返回
          if(empty() || head->next->next == head) 
            return;
          //桶排序:有64个桶 ---> 每个桶中存放的元素个数--> 2^n
          //取出list的第一个元素,放入第一个桶中
          //取出第二个,与一个做merge操作 ---> 局部有序链表 A---> 然后将其挪到第二个桶中
          //取出第三个,放到以第一个桶中
          //取出第四个,与第三个做merge操作 ---> 局部有序链表B --> A与B做merge操作 ---> 局部有序链表C ---> 存放到第三个桶中
          //局部有序性成倍地增加
          list<T,Alloc> tmp;   //中转
          list<T,Alloc> buckets[64];
          int fill = 0;            //记录已经填充了哪个桶
          while(!empty()){
            
            //将list的首元素转交给tmp;
            tmp.splice(tmp.begin(),begin());
            int i =0;
            //遍历buckets,若该bucket有元素,则将tmp的元素与其做merge操作 ---> 有序性倍增 ---> 应该放在后面的bucket中---> 移交给tmp ----> 稍后tmp在将该局部有序链表放到正确的bucket里
            while(i<fill && !buckets[i].empty()){
                buckets[i].merge(tmp);
                tmp.swap(buckets[i++]);
            }
            //移交到正确的bucket中
            tmp.swap(buckets[i]);
            
            if(i == fill)
              ++fill;
            //合并所有的局部有序链表
            for(int i=1;i<fill;i++)
              buckets[i].merge(buckets[i-1]);
            swap(buckets[fill-1]);
          }
        }

        //算法的自定义比较器版本 xxx_if
        template<class Predicate>
        void remove_if(const T& v,Predicate pred){
            iterator first = begin();
            while(first != end()){
              if(pred(*first,v))
                first = erase(first);
              else 
                ++first;
            }
            
        }
        template<class BinaryPredicate>
        void unique(BinaryPredicate binary_pred){
            if(empty())
              return;
            iterator first = begin();
            iterator next = first;
            while(++next != end()){
              if(binary_pred(*next == *first))
                erase(next);
              else 
                first = next;
              next = first;
            }
        }
        template<class StrictWeakOrdering>
        void merge(list& x,StrictWeakOrdering comp){
          iterator f1 = begin();
          iterator l1 = end();
          iterator f2 = x.begin();
          iterator l2 = x.end();
          iterator tmp;
          while(f1 != l1 && f2 != l2)
            if(comp(*f2 < *f1)){
              tmp = f2;
              transfer(f1,f2,++tmp);
              f2 = tmp;
            }else ++f1;
          if(l2 != f2) //链表x过长,则将其合并到尾部
            transfer(l1,l2,f2);
        }
        
        template<class StrictWeakOrdering>
        void sort(StrictWeakOrdering comp){
          //0或1个节点时,直接返回
          if(empty() || head->next->next == head) 
            return;
          list<T,Alloc> tmp;   
          list<T,Alloc> buckets[4];
          int fill = 0;            
          while(!empty()){
            tmp.splice(tmp.begin(),begin());
            int i =0;
            while(i<fill && !buckets[i].empty()){
                buckets[i].merge(tmp,comp);
                tmp.swap(buckets[i++]);
            }
            //移交到正确的bucket中
            tmp.swap(buckets[i]);
            
            if(i == fill)
              ++fill;
            //合并所有的局部有序链表
            for(int i=1;i<fill;i++)
              buckets[i].merge(buckets[i-1],comp);
            swap(buckets[fill-1]);
          }
        }
    };
}

#endif
