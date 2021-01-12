#pragma once 

#include <list>
#include <mutex>
#include "../datastructrue/ringbuffer.h"
#include "channelimpl.h"
#include "../common/co_condition.h"

namespace tiger{

  //通过 条件变量+环形队列实现 channnel
  template<class T>
    class LockedChannelImpl : public ChannelImpl<T>{
      typedef std::mutex LockType;
      typedef FastSteadyClock::time_point TimePoint;
      typedef CoConditionVar<T*> CondVar;
      typedef typename CoConditionVar<T*>::Status Status;

      LockType m_lock;
      //读写有各自的条件变量
      CondVar write_cond;
      CondVar read_cond;

      //若不使用环形队列，就是用双向链表
      bool use_ring_buffer;
      RingBuffer<T> m_bufffer;
      std::list<T> m_list;

      bool closed;

      const std::size_t m_capacity;


      //ctor
      public:
      explicit LockedChannelImpl(size_t capacity,bool use_buffer):m_capacity(capacity),closed(false),use_ring_buffer(use_buffer),m_bufffer(use_buffer ? capacity : 1){
          write_cond.set_relock_after_wait(false);
      }

      ~LockedChannelImpl(){}

      public:

      size_t size() const {
        return use_ring_buffer ? m_bufffer.size() : m_list.size();
      }
      bool empty() const {
        return size() == 0;
      }

      void close(){
        std::unique_lock<LockType> lock_guard(m_lock);
        if(closed)
          return ;
        closed = true;   //防止重复关闭 ---> 多个

        //唤醒所有阻塞的条件变量
        read_cond.notify_all();
        write_cond.notify_all();
      }

      bool push_impl(const T& t){
        if(use_ring_buffer)
          return m_bufffer.push(t);
        else{
          if(m_list.size() >= m_capacity())
            return false;
          m_list.emplace_back(t);
          return true;
        }
      }

      bool pop_impl(T& t){
        if(use_ring_buffer)
          return m_bufffer.pop(t);
        else{
          if(m_list.empty())
            t = m_list.front();
            m_list.pop_front();
            return true;
        }

      }

    
      bool push(T t, bool wait, FastSteadyClock::time_point deadline = 
                                    FastSteadyClock::time_point{}) override{
        if(closed)
          return false;
        //写入前先加锁
        std::unique_lock<LockType> lock_guard(m_lock);
        //二次判断
        if(closed)
          return false;
        
        //容量为0 ？？？？？？
        if(!m_capacity && read_cond.notify_one([&](T* p){*p = t;}))
          return true;
        //压入队列
        if(m_capacity>0 && push_impl(t)){ //队列已满就push失败
          if(size() == 1){ //从 0->1 通知读者
            //wait向Entry中写入 输出地址 --> value 
            //notify的lambda接受地址，为其指向的对象写入数据
            read_cond.notify_one([&](T* p){pop_impl(*p);});
          }
          return true;
        }

        if(!wait)  
          return false;

        
        Status m_status;
        if(deadline == TimePoint())  //立刻阻塞
          m_status = write_cond.wait(m_lock,&t);
        else 
          m_status = write_cond.wait_until(m_lock,deadline,&t);

        
        //查看是否超时?
        switch ((int)m_status){
          case (int)Status::NO_TIMEOUT:
            //管道已关闭
            if(closed)
              return false;
            return true;

          case (int)Status::TIMEOUT:
            return false;
          default:
            break;
        }
        return false;
      }
  //read
    bool pop(T & t, bool wait, FastSteadyClock::time_point deadline = FastSteadyClock::time_point{})override{
        if(closed)
          return false;
        //读取前先加锁
        std::unique_lock<LockType> lock_guard(m_lock);
        //二次判断
        if(closed)
          return false;
        if(m_capacity > 0){
          if(pop_impl(t)){   //队列中没有数据时。返回false
            if(size() == m_capacity-1)
              write_cond.notify_one([&](T* p){push(*p);});
            return true;
          }   
        }else{  //capacity == 0
          //若没有阻塞的写者，则notify_one返回false
          if(write_cond.notify_one([&](T* p){push(*p);}))
            return true;
        }
        if(!wait)
          return false;

        Status m_status;
        //阻塞在条件变量上 --> 看是否有时间限制
        if(deadline == TimePoint())
          m_status = read_cond.wait(m_lock,&t);   //notify_one唤醒时，会将数据写入Entry.value --> 这里通过&t指针接收
        else 
          m_status = read_cond.wait_until(m_lock,deadline,&t);
        //唤醒以后
        switch((int)m_status){
          case (int)Status::NO_TIMEOUT:
            if(closed)
              return false;
            return true;
          case (int)Status::TIMEOUT:
            return false;
          default:
            break;
        }
        return false;
  }

    };

}
