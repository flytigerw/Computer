
#pragma once 

#include <sys/types.h>
#include <memory>


namespace tiger{

  //环形队列 ---> 用作缓冲区
  //基于数组实现
  
  template<class T>
    class RingBuffer{

      private:
        T* m_buf;
        size_t m_capacity;
        size_t write=0;
        size_t read=0;

        size_t write_idx(){
          return write % m_capacity ;
        }
        size_t read_idx(){
          return read % m_capacity;
        }

        //ctor
      public:
        explicit RingBuffer(size_t capacity):m_capacity(capacity){
          m_buf = (T*) malloc(sizeof(T)*capacity);
        }

        ~RingBuffer(){
          T t;
          while(pop(t));  //先清除元素
          free(m_buf);
        }


      public:
        size_t size() const {
          return write - read;
        }
        size_t empty() const {
          return size() == 0;
        }
        size_t capacity() const{
          return m_capacity;
        }
        size_t full() const {
          return size() = capacity();
        }
        
        template<class Arg>     //移动写入
          bool push(Arg && arg){
            if(full())
              return false;

            T* ptr = m_buf + write_idx();
            
            //采用placement new进行构造
            new(ptr)T(std::forward<Arg>(arg));
            ++write;
            if(write == 0){

            }
            return true;
          }

        //引用返回
        template<class Arg>
          bool pop(Arg& arg){
            if(empty())
              return false;
            T* ptr = m_buf + read_idx();
            arg = std::move(*ptr);
            ptr->~T();   //已经move了
            ++read;
            return true;
          }



    };


}
