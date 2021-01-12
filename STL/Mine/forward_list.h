
#ifndef SLIST
#define SLIST 

#include "alloc.h"
#include "construct.h"

namespace stl{
  
  //类似rb_tree,迭代器也采取双层结构
  //结构部分
  struct slist_node_base{
    slist_node_base* next;
  };

  //将一个新节点插入在prev_node的后面
  inline slist_node_base* slist_make_link(slist_node_base* prev_node,slist_node_base* new_node){
      new_node->next = prev_node->next;
      prev_node->next = new_node;
      return new_node;
  }

  //返回在单向链表中，节点A的前一个节点
  inline slist_node_base* slist_previous_node(slist_node_base* head,const slist_node_base* node){
    while(head && head->next != node)
      head = head->next;
    return head;
  }
  //支持常量head
  inline const slist_node_base* slist_previous_node(const slist_node_base* head,const slist_node_base* node){
    while(head && head->next != node)
      head = head->next;
    return head;
  }
  
  //将范围[first,last)插入到pos的后面
  inline void slist_splice_after(slist_node_base* pos,slist_node_base* before_first,slist_node_base* before_last){
    before_first->next = before_last->next;
    before_last->next = pos->next;
    pos->next  = before_first->next;;
  }

  //将节点n后面的节点逆转
  inline slist_node_base* slist_reverse(slist_node_base* n){
    slist_node_base* last,*cur,*next;
    cur = n->next;
    last = n;
    while(cur){
      next = cur->next;
      cur->next = last;
      last = cur;
      cur = next;
    }
    return cur;
  }

  //泛化的数据
  template<class T>
    struct slist_node{
      T data;
    };

  //迭代器基类
  struct slist_iter_base{
    //前向迭代器
    typedef forward_iterator_tag iterator_category;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;
    slist_node_base* node_ptr;
    slist_iter_base(slist_node_base* x) : node_ptr(x){}
    void incur(){node_ptr = node_ptr->next;}
    bool operator==(const slist_iter_base& x){
      return x.node_ptr == node_ptr;
    }
    bool operator!=(const slist_iter_base& x){
      return x.node_ptr != node_ptr;
    }
  };

  template<class T,class Ref,class Pointer>
  struct slist_iter : public slist_iter_base{
      typedef slist_iter<T,T&,T*> iterator;
      typedef slist_iter<T,const T&,const T*> const_iterator;
      typedef slist_iter<T,Ref,Pointer> self_type;
      typedef T value_type;
      typedef Pointer pointer;
      typedef Ref reference;
      typedef slist_node<T> data_node;
      //ctor 
      slist_iter(data_node* x) : slist_iter_base(x){}
      slist_iter():slist_iter_base(nullptr){}
      slist_iter(const iterator&x) : slist_iter_base(x.data){}

      //操作符
      reference operator*()const{
        return ((data_node*)node_ptr)->data;
      }
      pointer operator->()const{
        return &(operator*());
      }
      //前置++
      self_type& operator++(){
        incur();
        return *this;
      }
      self_type operator++(int){
        auto tmp = *this;
        incur();
        return tmp;
      }
  };

  //节点n后面还有多少个节点
  inline size_t slist_size__(slist_node_base* n){
      size_t result = 0;
      for(;n;n=n->next)
        ++result;
      return result;
  }

  template<class T,class Alloc = alloc>
    class slist{
        public:
          //容器内建类型定义
          typedef T value_type;
          typedef value_type* pointer;
          typedef value_type& reference;
          typedef const value_type* const_pointer;
          typedef const value_type& const_reference;
          typedef size_t size_type;
          typedef ptrdiff_t difference_type;
          //迭代器定义
          typedef slist_iter<T,T&,T*> iterator;
          typedef slist_iter<T,const T&,T*> const_iterator;
        
        private:
          //类型定义，方便使用
          typedef slist_node<T> data_node;
          typedef slist_node_base  base_bode;
          typedef simple_alloc<T,alloc> slist_node_allocator;
          //内存相关
          //分配一个节点内存，并构造
          static data_node* new_node(const value_type& v){
            data_node* p = slist_node_allocator::allocate();
            try{
              construct(&p->data,v);
              p->next = nullptr;
            }catch(...){
              slist_node_allocator::deallocate(p);
              throw;
            }
            return p;
          }

          //销毁并释放节点
          static void delete_node(data_node* p){
            destory(p);
            slist_node_allocator::deallocate(p);
          }
          
          //分配n个单位的空间,并用n个v fill
          void fill_initialize(size_type n,const value_type& v){
            head.next = nullptr;
            try{
              insert_after_fill(&head,n,v);
            }catch(...){
              throw;
            }
          }
          //分配范围空间,并用迭代器范围构造
          template<class InputIterator>
            void range_initialize(InputIterator first,InputIterator last){
              
            head.next = nullptr;
            try{
              insert_after_range(&head, first, last);
            }catch(...){
              throw;
            }
          }
            
          //data member
        private:
          base_bode head;
          //辅助函数
        private:
          //在pos后面链接n个节点
          void insert_after_fill(base_bode* pos,size_type n,const value_type& v){
            //创建新节点，链接上去即可
            for(size_type i=0;i<n;i++)
              pos = slist_make_link(pos,new_node(v));
          }
          template<class InputIterator>
            void insert_afer_range(base_bode* pos,InputIterator first,InputIterator last){
              while(first != last){
                pos = slist_make_link(pos,new_node(*first));
                ++first;
              }
              
            }
          //新插入一个节点
          data_node* insert_after_(base_bode* pos,const value_type& v){
            return (data_node*)slist_make_link(pos,new_node(v));
          }
          //删除pos之后的一个节点,返回下一个iter
          base_bode* erase_after(base_bode* pos){
            auto tmp = (data_node*)(pos->next);
            auto iter = tmp->next;
            pos->next = tmp->next;
            destory(tmp);
            return iter;
          }
          //删除迭代器范围内的元素
          base_bode* erase_after(base_bode* before_first,base_bode* last){
            auto cur = (data_node*)(before_first->next);
            data_node* tmp;
            while(cur != last){
              tmp = cur;
              cur = (data_node*)cur->next;
              destory(tmp);
            }
            before_first->next = last;
            return last;
          }
          //构造函数
        public:
          slist(){head.next = nullptr;}
          slist(size_type n, const value_type& x) { fill_initialize(n, x); }
          explicit slist(size_type n) { fill_initialize(n, value_type()); }
          template<class InputIterator>
            slist(InputIterator first,InputIterator last){
              range_initialize(first,last);
            }
          //copy ctor 
          slist(const slist& l){
            range_initialize(l.begin(),l.end());
          }
          ~slist(){
            clear();
          }
          //对外接口
        public:
         iterator begin() { return iterator((data_node*)head.next); }
         const_iterator cbegin() const { return const_iterator((data_node*)head.next);}
         iterator end() { return iterator(0); }
         const_iterator cend() const { return const_iterator(0); }

         size_type size() const { return slist_size__(head.next); }
  
         size_type max_size() const { return size_type(-1); }

         bool empty() const { return head.next == 0; }
        
         //交换底层head的next
         void swap(slist& l){
            auto tmp = l.next;
            l.head.next = head.next;
            head.next = tmp;
         }

        public:
         reference front(){
           return ((data_node*)head.next)->data;
         }
         const_reference front() const{
           return ((data_node*)head.next)->data;
         }
         void pop_front(){
           auto tmp = (data_node*)head.next;
           head.next = tmp->next;
           delete_node(tmp);
         }
         //pos的iter
         iterator previous(const_iterator pos){
           return iterator((data_node*)slist_previous_node(&head,pos.node_ptr));
         }
        
         //插入系列
        public:
         iterator insert_after(iterator pos,const value_type& x){
           return iterator(insert_after_(pos.node_ptr,x));
         }
         iterator insert_after(iterator pos){
           return iterator(insert_after_(pos.node_ptr,value_type()));
         }
         void insert_after(iterator pos, size_type n, const value_type& x) {
           insert_after_fill(pos.node_ptr,n,x);
         }
        template <class InputIterator>
        void insert_after(iterator pos, InputIterator first, InputIterator last) {
          insert_after_range(pos.node, first, last);
         }
        //通用insert接口
        iterator insert(iterator pos, const value_type& x)      {
         return iterator(insert_after_(slist_previous_node(&head, pos.node), x));
         }

        iterator insert(iterator pos) {
         return iterator(nsert_after_(slist_previous_node(&head, pos.node),value_type()));
         }


        void insert(iterator pos, size_type n, const value_type& x) {
          insert_after_fill(slist_previous_node(&head, pos.node), n, x);
         } 
        void insert(iterator pos, int n, const value_type& x) {
          insert_after_fill(slist_previous_node(&head, pos.node), (size_type) n, x);
         } 
        void insert(iterator pos, long n, const value_type& x) {
          insert_after_fill(slist_previous_node(&head, pos.node), (size_type) n, x);
         } 
        template <class InIter>
         void insert(iterator pos, InIter first, InIter last) {
          insert_after_range(slist_previous_node(&head, pos.node), first, last);
         }
        //删除系列
        iterator erase_after(iterator pos) {
         return iterator((data_node*)erase_after(pos.node_ptr));
         }
        iterator erase_after(iterator before_first, iterator last) {
         return iterator((data_node*)erase_after(before_first.node_ptr, last.node_ptr));
         }

        iterator erase(iterator pos) {
         return (data_node*) erase_after(slist_previous_node(&head, pos.node_ptr));
        }
        iterator erase(iterator first, iterator last) {
         return (data_node*) erase_after(slist_previous_node(&head, first.node_ptr),last.node_ptr);
        }
        void resize(size_type new_size,const T&x){
          //先遍历到new_size处，再选择插入或者删除
          base_bode* cur = &head;
          while(cur && new_size>0){
            cur = cur->next;
            --new_size;
          }
          if(cur->next)
            erase_after(cur,nullptr);
          else 
            insert_after_fill(cur,new_size,x);
       }
       void resize(size_type new_size){
        resize(new_size,value_type());
       }
       void clear(){
        erase_after(&head,nullptr);
       }
       //自由算法
        public:
        //将[first,last)转移到pos后面
       void splice_after(iterator pos,iterator before_first,iterator before_last){
         if(before_first != before_last)
          slist_splice_after(pos.node_ptr,before_first.node_ptr,before_last.node_ptr);
       }
       //将一个元素插入到pos后面
       void splice_after(iterator pos,iterator before_first){
         slist_splice_after(pos,before_first,before_first->next);
       }
       //将链表l转移到当前链表的x后面
       void splice(iterator pos,slist& l){
         if(l.head.next)
           slist_splice_after(slist_previous_node(&head,pos.node_ptr),&l.head,slist_previous_node(&l.head,nullptr));
       }
       //将链表l的i节点都转移到pos后面
       void splice(iterator pos,slist& l,iterator i){
         slist_splice_after(slist_previous_node(&head,pos.node_ptr),slist_previous_node(&l.head,i.node_ptr),i.node_ptr);
       }
       //将链表l的[first,last)转移到pos后面
       void splice(iterator pos,slist& l,iterator first,iterator last){
         if(first != last){
            slist_splice_after(slist_previous_node(&head,pos.node_ptr),slist_previous_node(&l.head,first.node_ptr),slist_previous_node(&l.head,last.node_ptr));
         }
       }

       void reverse(){
         if(head.next)
           head.next = slist_reverse(head.next);
       }


       //删除链表中所有值为val的node
       void remove(const T& val){
          base_bode* cur = &head;
          while(cur && cur->next){
            if(((data_node*)cur->next)->data == val)
              erase_after(cur);
            else 
              cur = cur->next;
          }
       }
        
       //删除链表相邻重复的节点
       void unique(){
          base_bode* cur = &head;
          if(cur){
            while(cur->next){
              if(((data_node*)cur)->data == ((data_node*)(cur->next))->data)
                erase_after(cur);
              else 
                cur = cur->next;
            }
          }
       }
    };
}

#endif
