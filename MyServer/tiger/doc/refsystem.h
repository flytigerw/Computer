

#pragma once 
#include <string.h>
#include <vector>
#include <stdint.h>
#include <atomic>

namespace tiger{


  //侵入式引用计数体系
  
  class RefCounter;

  //上层可指定析构器,下层回调之
  struct Deleter{
    using func_type = void(*)(RefCounter* p,void* arg);
    func_type deleter;
    void* args;

    //ctor
    Deleter(): deleter(nullptr),args(nullptr){}
    Deleter(func_type func, void* args_) : deleter(func), args(args_) {}

    //
    inline void operator()(RefCounter* ptr);
  };


  template<class T>
  class IncursivePtr;


  //用户类需继承之
  class RefCounter{

    template<class T>
    friend class IncursivePtr;

    private:
      //原子变量，保证并发安全性
      //count标识当前有多少IncursivePtr指向该资源
      std::atomic<long> count;
      std::atomic<long>* p_count;
      Deleter deleter;

    public:
      RefCounter() : count{0}{
        p_count = &count;
      }
      virtual ~RefCounter(){}

      //避免继承的用户类擅自调用引用计数器操作接口
      //导致引用计数器的不平衡
      //引用计数器接口只能由友元IncursivePtr调用
    private:
      void increment(){
        ++*p_count;
      }

      virtual bool decrement(){
        if(--*p_count == 0){
          deleter(this);
          return true;
        }
        return false;
      }

    public:
      long get_count() const{
        return *p_count;
      }
      void set_deleter(Deleter d){
        deleter = d;
      }
  };


  inline void Deleter::operator()(RefCounter* p){
    if(deleter)
      deleter(p,args);
    else 
      delete p;
  }

  //使用:IncursivePtr<T> p(new T)
  template<class T>
    class IncursivePtr{

      private:
        //指向继承了 RefCounter的 动态用户类资源
        T* ptr = nullptr;

      public:
        //用原始构造时，++count
        explicit IncursivePtr(T* p):ptr(p){
          if(ptr)
            ptr->increment();
        }
        //拷贝构造,增加共享人员,++count
        IncursivePtr(const IncursivePtr& p):ptr(p.ptr){
          if(ptr)
            ptr->increment();
        }

        //移动构造，共享人员不变
        IncursivePtr(IncursivePtr&& p){
          ptr = p.ptr;
          p.ptr = nullptr;
        }

        //析构，共享人员-1，若无人共享，则销毁资源
        ~IncursivePtr(){
          if(ptr)
            ptr->decrement();
        }

        void reset(){
          if(ptr){
            ptr->decrement();
            ptr = nullptr;
          }
        }

        //copy =
        IncursivePtr& operator=(const IncursivePtr& p){
          if(this == &p)
            return *this;
          //重置左边的
          reset();
          ptr = p.ptr;
          //增加右边资源的共享人数
          if(ptr)
            ptr->increment();
          return *this;
        }
        //move =
       IncursivePtr& operator=(IncursivePtr && other) {
         if (this == &other) 
           return *this;
         //重置左边的
         reset();
         //steal from right
         std::swap(ptr, other.ptr);
         return *this;
       }

       //资源访问
       T& operator*() const{
         return *ptr;
       }
       T* operator->() const{
         return ptr;
       }

       //指针用于条件判断
       //!ptr -> 将指针转为bool
       //! !ptr -> 反反得正
       explicit operator bool() const {
         return !!ptr;
       }

       //指针比较
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
}
