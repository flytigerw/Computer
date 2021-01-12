

#pragma once

#include <atomic>
#include <string.h>
#include <stdint.h>
#include "../../base.h"

namespace tiger{

  //半侵入式的引用计数体系，在使用shared_ptr语义的同时，又可以将对象放入到侵入时容器


  //用户自定义的侵入式对象需要继承RefCounter
  struct RefCounter;  //引用计数对象
  struct RefObjectImpl;
  struct SharedRefObject;

  //Deleter:当对象的引用计数为0时，调用Deleter进行析构
  struct Deleter{
    using func_type = void(*)(RefCounter* p,void* arg);
    func_type m_deleter;
    void* m_arg;

    //ctor 
    Deleter(): m_deleter(nullptr),m_arg(nullptr){}
    Deleter(func_type func, void* arg) : m_deleter(func), m_arg(arg) {}

    inline void operator()(RefCounter* ptr);
  };

  struct RefCounter : public Noncopyble{
    //同样引入指针提供一层间接性
    std::atomic<long>* p_count;
    std::atomic<long> count;
    Deleter m_deleter;

    //对象创建时，引用计数就为1
    RefCounter() : count{1}{
      p_count = &count;
    }
    virtual ~RefCounter(){}

    void increment(){
          ++*p_count;
    }

    //减到0 返回false
    virtual bool decrement(){
      if(--*p_count == 0){
        m_deleter(this);
        return true;
      }
      return false;
    }

    long get_count(){
      return *p_count;
    }

    //如果共享，则p_count指向WeakCounter的count
    bool is_shared(){
      return p_count != &count;
    }
    void set_deleter(Deleter d){
      m_deleter = d;
    }
  };

  inline void Deleter::operator()(RefCounter* ptr){
      if(m_deleter)
        m_deleter(ptr,m_arg);
      else 
        delete ptr;
    }

  //weak 用于WeakPtr时
  
  struct WeakCounter{
    std::atomic<long> ref_count;
    std::atomic<long> weak_count;          //弱引用计数

    WeakCounter() : ref_count{1},weak_count{1}{}

    void increment_weak(){
      ++weak_count;
    }

    void decrement_weak(){
      if(--weak_count == 0)
        delete this;
    }
    //主要用于转为强引用计数指针
    bool lock(){
      //多个WeakPtr同时转为IncursivePtr时就需要保护
      long x = ref_count.load(std::memory_order_relaxed);
      do{
        if(x == 0)   //何时为0  ---> decrement到0 --> decrement一半,被切换到lock
                    //不为0,可能存在多个WeakPtr调用lock的情况
          return false;
      }while(!ref_count.compare_exchange_weak(x,x+1,   //CAS操作,若ref_count的值就为x，ref_count++,返回true
//read:acquire语义 ---> 要等待release语义
            std::memory_order_acq_rel,                 
            std::memory_order_relaxed));
      return true;
    }
    
  };

  //可shared的引用计数器
  struct SharedRefCounter : public RefCounter{
    WeakCounter* m_weak_counter;
    
    SharedRefCounter() : m_weak_counter(new WeakCounter){
      this->p_count = &m_weak_counter->ref_count;
    }

    virtual bool decrement(){
      WeakCounter* w = m_weak_counter;
      //ref_count减到0，相应的WeakCounter也该delete
      if(RefCounter::decrement()){
            std::atomic_thread_fence(std::memory_order_acq_rel); //可以防止decrement_weak重排到decrement前.为什么要这么做呢?
            
            w->decrement_weak();
            return true;
      }
      return false;
    }
  };
  //侵入式地引用计数智能指针:侵入式对象本身包含引用计数器
  //该指针值是提供操作
  template<class T>
    class IncursivePtr{
      private:
        T* ptr = nullptr;
      public:
        //构造时引用计数+1
        explicit IncursivePtr(T* p):ptr(p){
          if(ptr)
            ptr->increment();
        }
        //析构时，引用计数-1
        ~IncursivePtr(){
          if(ptr)
            ptr->decrement();
        }
        IncursivePtr(const IncursivePtr& p):ptr(p.ptr){
          if(ptr)
            ptr->increment();
        }
        IncursivePtr(IncursivePtr&& p){
          ptr = p.ptr;
          p.ptr = nullptr;
        }
        void reset(){
          if(ptr){
            ptr->decrement();
            ptr = nullptr;
          }
        }
        IncursivePtr& operator=(const IncursivePtr& p){
          if(this == &p)
            return *this;
          reset();
          ptr = p.ptr;
          if(ptr)
            ptr->increment();
          return *this;
        }
       IncursivePtr& operator=(IncursivePtr && other) {
         if (this == &other) 
           return *this;
         reset();
         std::swap(ptr, other.ptr_);
         return *this;
       }
      T& operator*() const { return *ptr; }
      T* operator->() const { return ptr; }
      //转为bool值
      explicit operator bool() const { return !!ptr; }
      T* get() const { return ptr; }


      long get_count(){
        return ptr ? (long)*ptr->p_count : 0;
      }

      bool unique(){
        return  get_count() == 1;
      }
      //地址比较
      void swap(IncursivePtr & other) {
        std::swap(ptr, other.ptr);
      }
      bool operator==(const IncursivePtr& rhs) {
        return ptr == rhs.ptr;
      }
      bool operator!=(const IncursivePtr& rhs) {
        return ptr != rhs.ptr;
      }
      bool operator<(const IncursivePtr& rhs) {
        return ptr < rhs.ptr;
      }
    };

  //保证在析构时递减引用计数
  template<class T>
    class AutoReleasePtr{

      public:
      explicit AutoReleasePtr(T* ptr) : ptr(ptr) {}
      void release() {
        ptr = nullptr;
      }

      ~AutoReleasePtr() {
        if (ptr) 
          ptr->decrement();
      }

      private:
        T* ptr;
    };

// 弱指针
// 注意：弱指针和对象池不可一起使用, 弱指针无法区分已经归还到池的对象.
  //用户提供的T对象需要继承SharedRefCounter
  //WeakPtr的特点:指示对象的引用计数，但不管理对象的生命周期
  template<class T>
    class WeakPtr{
      private:
        WeakCounter* m_weak_counter = nullptr;
        T* ptr = nullptr;

      public:
        WeakPtr(){}
        explicit WeakPtr(T* p){
          reset(p);
        }
      
        //用强引用计数指针构造弱引用计数指针
        //获取底层的对象指针 ---> 构造弱引用计数指针
       explicit WeakPtr(const IncursivePtr<T>& iptr){
        T* ptr = iptr.get();
        reset(ptr);
       }
       //析构时，只是递减引用计数 ---> 不控制资源
       ~WeakPtr() {
        reset();
       }
       //拷贝构造时，递增弱引用计数
       WeakPtr(WeakPtr const& other) : m_weak_counter(other.m_weak_counter), ptr(other.ptr) {
        if (m_weak_counter) 
          m_weak_counter->increment_weak();
      }
      WeakPtr(WeakPtr && other){
       swap(other);
      }
      WeakPtr& operator=(WeakPtr && other) {
        swap(other);
        return *this;
      }
      
      //拷贝=,先reset()清空
      WeakPtr& operator=(const WeakPtr& other){
        if(this == &other)
          return *this;
        reset();
        if(other.m_weak_counter){
          m_weak_counter = other.m_weak_counter;
          ptr = other.ptr;
          m_weak_counter->increment_weak();
        }
        return *this;
      }
      void swap(WeakPtr & other) {
        std::swap(m_weak_counter, other.m_weak_counter);
        std::swap(ptr, other.ptr);
      }

      void reset(){
        if(m_weak_counter){
          m_weak_counter->decrement_weak();
          m_weak_counter = nullptr;
          ptr = nullptr;
        }
      }

        //重新指向新的对象
      void reset(T* p){
        reset();
        if(!p)
          return;
        if(!p->is_shared())
          return;
                          //转为基类,获取weak_counter
        m_weak_counter = ((SharedRefCounter*)p)->m_weak_counter;
        ptr = p;
       //弱引用计数增加
        m_weak_counter->increment_weak();
      }
      explicit operator bool() const {
        return !!m_weak_counter;
      }

      //转为强引用指针
      IncursivePtr<T> lock() const{
        if(!m_weak_counter)
          return IncursivePtr<T>();
        if(!m_weak_counter->lock())
          return IncursivePtr<T>();
        IncursivePtr<T> res(ptr); //强引用计数+1
        //弱引用计数-1 
        ptr->decrement();
        return res;
      }
      bool operator==(const WeakPtr<T>& rhs) const {
        return m_weak_counter == rhs.m_weak_counter && ptr == rhs.ptr;
    }

      bool operator<(const WeakPtr<T>& rhs) const {
        return m_weak_counter < rhs.m_weak_counter;
    }

    long use_count() {
        return m_weak_counter ? (long)m_weak_counter->ref_count : 0;
    }
    };

  //将侵入式对象的裸指针转为非侵入式智能指针
  template<typename T>
    typename std::enable_if<std::is_base_of<RefCounter,T>::value,std::shared_ptr<T>>::type 
    shared_from_this(T* ptr){
      //对象本身的引用计数+1
      ptr->increment();
      //析构时只要--对象内部的引用计数即可
      return std::shared_ptr<T>(ptr,[](T* p){p->decrement();});
    }
  template<typename T,typename ...Args>
    typename std::enable_if<std::is_base_of<RefCounter,T>::value,std::shared_ptr<T>>::type 
    make_shared(Args&& ...args){
      T* ptr = new T(std::forward<Args>(args)...);
      return std::shared_ptr<T>(ptr,[](T* p){p->decrement();});
    }
  //方便使用的函数 ---> 对外接口，保证对象必须继承RefCounter
  template<typename T,typename ...Args>
    typename std::enable_if<std::is_base_of<RefCounter,T>::value,std::shared_ptr<T>>::type 
    Increment(T* ptr){
      ptr->increment();
    }
  template <typename T>
    typename std::enable_if<!std::is_base_of<RefCounter, T>::value>::type
    Increment(T * ptr){
    //do nothing
    }
  template<typename T,typename ...Args>
    typename std::enable_if<std::is_base_of<RefCounter,T>::value,std::shared_ptr<T>>::type 
    Decrement(T* ptr){
      ptr->decrement();
    }
  template <typename T>
    typename std::enable_if<!std::is_base_of<RefCounter, T>::value>::type
    Decrement(T * ptr){
    //do nothing
    }


    //引用计数器guard ---> 类似lock和lockguard
    //在构造时递增引用计数
    //在析构时递减引用计数
  class RefGuard : public Noncopyble{
    private:
      RefCounter* ptr;
    public:
      //转为基类
      template<class T>
        explicit RefGuard(T* p):ptr(static_cast<RefCounter*>(p)){
          ptr->increment();
        }

      template<class T>
        explicit RefGuard(T& obj):ptr(static_cast<RefCounter*>(&obj)){
          ptr->increment();
        }
      ~RefGuard(){
        ptr->decrement();
      }
  };
  

  //全局对象计数器:维护静态原子变量
  //采用模板:每种全局对象都可以有一个计数器
  template<class T>
    class GlobalObjectCounter{

      //构造时+1,析构时-1
      GlobalObjectCounter(){++global_counter();}
      GlobalObjectCounter(const GlobalObjectCounter&){++global_counter();}
      GlobalObjectCounter(GlobalObjectCounter&&){++global_counter();}
      ~GlobalObjectCounter(){
        --global_counter();
      }
      //对外接口
      static long get_global_count(){
            return global_counter();
      }
      private:
        //私有构函数
        static std::atomic<long>& global_counter(){
          static std::atomic<long> c;
          return c;
        }
    };

  //ID counter
template <typename T>
struct IDCounter
{
    IDCounter() { id = ++counter(); }
    IDCounter(IDCounter const&) { id = ++counter(); }
    IDCounter(IDCounter &&) { id = ++counter(); }

    long getID() const {
        return id;
    }

private:
    static std::atomic<long>& counter() {
        static std::atomic<long> c;
        return c;
    }
    long id;
};


//创建Fiber的源码文件位置:文件+行号
//Bean类
struct SourceLocation{

  void set(const char*file,int line){
    m_file = file;
    m_line = line;
  }

  //提供< ---> 需要使用map存储
   bool operator<(const SourceLocation& rhs)
   {

      if (m_line != rhs.m_line)
          return m_line < rhs.m_line;
      if (m_file == rhs.m_file) 
        return false;
      if (m_file == nullptr)
          return true;
      if (rhs.m_file == nullptr)
          return false;
      return strcmp(m_file, rhs.m_file) == -1;
    }
    std::string ToString() const
    {
        std::string s("{file:");
        if (m_file) 
          s += m_file;
        s += ", line:";
        s += std::to_string(m_line) + "}";
        return s;
    }
  
  const char* m_file = nullptr;
  int m_line = 0;
};

}
