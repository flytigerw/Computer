#pragma once 


#include <list>
#include <sys/types.h>
#include <mutex>
#include "../common/refsystem.h"
#include "../common/lock.h"
#include "../../base.h"
#include "../../macro.h"

namespace tiger{

  //双向链表节点的结构，不包含数据
  struct ListNode{ 
    ListNode* prev = nullptr,*next = nullptr;
      //追加新节点到链表末尾
    void link(ListNode* theNext) {
        MY_ASSERT(next == nullptr);
        MY_ASSERT(theNext->prev == nullptr);
        next = theNext;
        theNext->prev = this;
    }

    //将链表断开
    void unlink(ListNode* theNext){
        MY_ASSERT(next == theNext);
        MY_ASSERT(theNext->prev == this);
        next = nullptr;
        theNext->prev = nullptr;
    }
  };

  /* 用户自定义class : public ListNode
   * 在SList管理下，多个ListNode串成双向链表，SList也提供相关的链表操作
   * 即:用户给出给定结构(与SList约定好)的数据，SList提供方法，维护这些数据
   *
   * */
  /* 插入:append() --> 只需要尾后追加
   * 删除:erase(T*)
   * cut(size_t n) --> 将前n个切割出来
   * 有点类似队列
   * */
  //以ListNode为基础的双向链表
  template<class T>
    class SList : public Noncopyble{
      //用户必须提供给定结构的数据
      static_assert((std::is_base_of<ListNode,T>::value),"T Must be base of ListNode");
      
      public:
      struct iterator{
        //三根指针的迭代器:支持边遍历，边删除
        T* cur,*next,*prev;
        iterator() : cur(nullptr), prev(nullptr), next(nullptr) {}

        iterator(T* p) { 
          locate(p); 
        }

        //定位到某一节点
        void locate(T* p){
          cur = p;
          next = cur ? (T*)cur->next : nullptr;
          prev = cur ? (T*)cur->prev : nullptr;
        }
        iterator& operator++(){
          locate(next);
          return *this; 
        }
        iterator& operator++(int){
          auto tmp = *this;
          ++(*this);
          return tmp; 
        }
        iterator& operator--(){
          locate(prev);
          return *this; 
        }
        iterator& operator--(int){
          auto tmp = *this;
          --(*this);
          return tmp; 
        }
        T& operator*() {
          return *(T*)cur; 
        }
        T* operator->() {
          return (T*)cur;
        }
        bool operator==(const iterator& rhs) const {
          return cur == rhs.cur; 
        }
        bool operator!=(const iterator& rhs) const{
          return !(*this == rhs); 
        }
      };

      //数据成员:头尾指针 + 元素个数
      public:
       T* head,*tail;
       size_t count;
       SList() : head(nullptr), tail(nullptr), count(0) {}

       SList(ListNode* h, ListNode* t,size_t c)
        : head((T*)h), tail((T*)t), count(c) {}

       SList(ListNode* h)
        : head((T*)h), tail((T*)h), count(1) {}

      //移动构造:指针移动即可
       void reset(){
         head = tail = nullptr;
         count  = 0;
       }

       SList(SList<T>&& slist){
         head =  slist.head;
         tail =  slist.tail;
         count  = slist.count;
         slist.reset();
       }

       //移动赋值
       SList& operator=(SList<T>&& slist){
         //清除原来的
         clear();
         head =  slist.head;
         tail =  slist.tail;
         count  = slist.count;
         slist.reset();
         return *this;
       }
       
       //末尾追加链表:移动语义
       void append(SList<T>&& slist){
         if(slist.empty())
           return;
         if(empty()){
            *this = std::move(slist);
            return;
         }
         //将slist的头部追加到tail后面即可
         tail->link(slist.head);
         tail = slist.tail;
         count += slist.count;
         slist.reset();
       }


       //把前n个切割出去
       SList<T> cut(size_t n){
         if(empty())
           return SList<T>();
         if(n >= size()){
            SList<T> o(std::move(*this));
            return o;
         }
         if(n == 0)
           return SList<T>();

         SList<T> res;
         auto it = head;
         //定位到第n个节点
         for(size_t i=0;i<n;i++)
           it= (T*)it->next;
         res.head = head;
         res.tail = it;
         res.count = n;
         count -= n;
         head = (T*)it->next;
         it->unlink(head);

         //移动语义
         return res;
       }

      public:
       iterator begin(){
         return iterator(head);
       }
       iterator end(){ 
         return iterator(); 
       }

       void erase(T* p){
          if(p->prev)
            p->prev->next = p->next;
          else  //删除头结点
            head = (T*)head->next;
          if(p->next)
            p->next->prev = p->prev;
          else 
            tail = (T*)tail->prev;
          p->next = p->prev = nullptr;
          --count;
          //递减引用计数 ---> 从容器删除的必要操作
          Decrement(p);
       }

       bool erase(T* ptr,void* check){
       // if (ptr->check_ != check) 
        //  return false;
        erase(ptr);
        return true;
       }
       iterator erase(iterator it){
         //返回下一个节点
         T* p = (it++).ptr;
         erase(p);
         return it;
       }

       void clear(){
         auto it = begin();
         while(it != end())
           it = erase(it);
         reset();
       }

       bool empty(){
         return head = nullptr;
       }
       size_t size() const {
         return count;
       }
       ListNode* get_head(){
         return head;
       }
       ListNode* get_tail(){
         return tail;
       }
    };

    //线程安全的队列
    //用户可以选择是否开启线程安全
  template<class T,bool ThreadSafe = true>
    class TSQueue{
      static_assert((std::is_base_of<ListNode,T>::value),"T Must be base of ListNode");

      public:

      typedef typename std::conditional<ThreadSafe,LFLock,FakeLock>::type LockType;
      typedef std::unique_lock<LockType> LockGuard;

     //数据成员 
     //用于保护的lock 
      LockType lock,*p_lock; //指针的目的 ---> 引入一层间接性 ---> 底层的atomic_flag不支持= ---> 更换时可通过交换指针
     
      ListNode* head,*tail;
      volatile size_t count;   //保证对齐

      public:
      TSQueue(){
        head = tail = new ListNode;
        count = 0;
        p_lock = &lock;
      }
      ~TSQueue(){
        LockGuard lock_guard(*p_lock);
        while (head != tail) {
            ListNode *prev = tail->prev;
            Decrement((T*)tail);
            tail = prev;
        }
        delete head;
        head= tail = nullptr;
      }

      LockType& lock_ref(){
        return *p_lock;
      }

      void set_lock(LockType* p){
          p_lock = p;
      }
      //读写队列元素时，都先上锁
      //获取指定位置的下一个元素
      //通过引用返回
      void next(T* p,T*& out){
          LockGuard lock_guard(*p_lock);
          out = (T*)p->next;
      }
      void next_without_lock(T* ptr, T*& out){
        out = (T*)ptr->next;
      }

      //队首元素
      void front(T*& out){
          LockGuard lock_guard(*p_lock);
          out = (T*)head->next;
      }
      bool empty(){
        LockGuard lock_guard(*p_lock);
        return !count;
      }

      bool empty_without_lock(){
        return !count;
      }

      size_t size(){
        LockGuard lock_guard(*p_lock);
        return count;
      }

      //弹出队首元素
      T* pop(){
        //减少一次加锁
        if(head == tail)
          return nullptr;
        LockGuard lock_guard(*p_lock);
        if(head == tail)
          return nullptr;
        ListNode* p = head->next;
        if(p == tail)    
          tail = head;
        head->next = p->next;
        if(p->next)
          p->next->prev = head;
        p->prev = p->next = nullptr;
        --count;
        Decrement((T*)p);
        return (T*)p;
      }

      //在队尾移动追加多个元素:O(1)
      void push(SList<T>&& elements){
        if(elements.empty())
          return;
        LockGuard lock_guard(*p_lock);
        push_without_lock(std::move(elements));
      }
      //可扩展性
      void push_without_lock(SList<T>&& elements){
        if (elements.empty())
          return ;
        MY_ASSERT(elements.head->prev == nullptr);
        MY_ASSERT(elements.tail->next == nullptr);
        ListNode* the_head = elements.head;
        count += elements.size();
        tail->link(the_head);
        tail = elements.tail();
        elements.reset();
      }
      //区别在于refCount
      size_t push_without_lock(T* element, bool refCount = true)
    {
        ListNode* hook = static_cast<ListNode*>(element);
        tail->link(hook);
        tail = hook;
        hook->next = nullptr;
        ++ count;
        if (refCount)
            increment(element);
        return count;
    }
    
      //截取前n个
    SList<T> pop_front(uint32_t n){
        LockGuard lock_guard(*p_lock);
        if (head == tail) 
          return SList<T>();
        ListNode* first = head->next;
        ListNode* last = first;
        uint32_t c = 1;
        //找到目标位置
        for (; c < n && last->next; ++c) 
            last = last->next;

        if (last == tail) 
          tail = head;
        head->next = last->next;
        if (last->next) 
          last->next->prev = head;
        first->prev = last->next = nullptr;
        count -= c;
        return SList<T>(first, last, c);
    }

    //弹出后面的n个元素,返回给调用者
    SList<T> pop_back(uint32_t n){
        if (head == tail) 
          return SList<T>();
        LockGuard lock_guard(*p_lock);
        return pop_back_without_lock(n);
    }
    SList<T> pop_back_without_lock(uint32_t n){
        if (head == tail) 
          return SList<T>();
        ListNode* last = tail;
        ListNode* first = last;
        uint32_t c = 1;
        for (; c < n && first->prev != head; ++c) {
            first = first->prev;
        }
        tail = first->prev;
        first->prev = tail->next = nullptr;
        count-= c;
        return SList<T>(first, last, c);
    }
    //弹出所有,返回给调用中
    SList<T> pop_all(){
        if (head == tail) return SList<T>();
        LockGuard lock_guard(*p_lock);
        return pop_all_without_lock();
    }

    SList<T> pop_all_without_lock(){
        if (head == tail)
          return SList<T>();
        ListNode* first = head->next;
        ListNode* last = tail;
        tail = head;
        head->next = nullptr;
        first->prev = last->next = nullptr;
        std::size_t c = count;
        count = 0;
        return SList<T>(first, last, c);
    }

    //删除指定位置的元素
    bool erase(T* hook)
    {
        LockGuard lock_guard(*p_lock);
        return erase_without_lock(hook );
    }

    bool erase_without_lock(T* hook,bool refCount = true)
    {
        MY_ASSERT(hook->prev != nullptr);
        MY_ASSERT(hook == tail || hook->next != nullptr);
        if (hook->prev) 
          hook->prev->next = hook->next;
        if (hook->next)
          hook->next->prev = hook->prev;
        else if (hook == tail)
          tail = tail->prev;
        hook->prev = hook->next = nullptr;
        MY_ASSERT(count > 0);
        -- count;
        //如果元素含有引用计数，就递减
        if (refCount)
          Decrement((T*)hook);
        return true;
    }
    size_t push(T* element)
    {
        LockGuard lock_guard(*p_lock);
        return push_without_lock(element);
    }

    };


  //侵入式等待队列:底层为单向链表
  struct WaitNode{
      WaitNode* next;
  };

  template<class T>
    class WaitQueue{
      static_assert(std::is_base_of<WaitNode,T>::value,"T must inherer from WaitNode");

      public:
        typedef std::mutex LockType;
        LockType lock;
        //头尾指针
        WaitNode* head,*tail;
        //用作队列头，但不存储数据，指示作用
        WaitNode dummy;
        //数量
        volatile size_t size;

        const size_t blocking_thresh;
        

    explicit WaitQueue(){
        head = &dummy;
        tail = &dummy;
        size = 0;
    }

    size_t is_empty() const {return size == 0;}
    size_t get_size() const {return size;}

    void push(T* ptr){
      std::unique_lock<LockType> lock_guard(lock);
      //插入到队尾巴
      tail->next = ptr;
      ptr->next = nullptr;
      tail = ptr;
    }

    //弹出队首,引用返回队首
    bool pop(T*& ptr){
      std::unique_lock<LockType> lock_guard(lock);
      if(is_empty())
        return false;
      ptr = static_cast<T*>(head->next);
      //一个元素的情况
      if(tail == head->next)
        tail = head;
      head->next = head->next->next;
      ptr->next = nullptr;
      //将位于blocking_thresh后面的元素转为非阻塞态
      return true;
    }
  };


}
