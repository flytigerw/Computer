

#pragma once
#include <list>
#include <mutex>
#include "channelimpl.h"
#include "../common/co_condition.h"
#include "../scheduler/processor.h"
#include "../scheduler/scheduler.h"
#include <atomic>


namespace tiger{


  template<class T>
   class CASChannelImpl{
     private:
       struct Entry{  //存储在condition 的等待队列中
         int id;
         T* p_val;
         T val;
       };

     public:
      typedef std::mutex LockType;
      typedef FastSteadyClock::time_point TimePoint;
      typedef CoConditionVar<Entry> CondVar;
      typedef typename CondVar::Status Status;
      typedef typename CondVar::CondRet CondRet;

      CondVar write_cond,read_cond;
      bool closed;
      const size_t m_capacity;
      LockType m_lock;

      //读写等待的数量记录
      //前32位记录等待的read
      //后32位记录等待的write
      std::atomic<uint64_t> m_wait{0};
      static constexpr uint64_t write_mask =  0xffffffff;
      static constexpr uint64_t read_mask =  0xffffffff00000000;
      static constexpr uint64_t one_write = 1;
      static constexpr uint64_t one_read =((size_t)1) << 32;


      //自旋次数
      static constexpr int spin_count = 4000;

     public:
      explicit CASChannelImpl(size_t capacity):m_capacity(capacity),closed(false),write_cond(capacity ? capacity+1: size_t(-1),NULL){
      }

      ~CASChannelImpl(){}

     public:
      bool push(T t,bool time_wait,FastSteadyClock::time_point deadline = FastSteadyClock::time_point{}){

        if(closed)
          return false;

        int spin = 0;

        uint64_t wait;
        for(;;){
          wait = m_wait.load(std::memory_order_relaxed);
          //唤醒阻塞的读者。将数据写入读者的等待队列中
          if(wait & read_mask){ //阻塞的读者不为0
            if(!read_cond.notify_one(  //读者唤醒失败 continue
                  [&](Entry & entry){  //将t写入读者的等待队列  
                  *entry.p_val = t;
                  })){
              //自旋超时
              if(++spin >= spin_count){
                  spin = 0;
                  Processer::FiberYield();
              }
              //每次自旋时的检查
              if(m_capacity == 0){
                if(closed)
                  return false;
                else if(!time_wait)
                  return false;
              }
              continue;
            }
            //减少一个阻塞的读者
            m_wait -= one_read;
          }else if(m_capacity == 0){
                if(closed)
                  return false;
                else if(!time_wait)
                  return false;
          }
          //没有阻塞的写者
         if(m_wait.compare_exchange_weak(wait,wait+one_write,std::memory_order_acq_rel,std::memory_order_relaxed))
          break;
          
        }

        FakeLock lock;
        Entry entry;
        //entry.id 
        entry.val = t;
        auto cond = [&](size_t size)->CondRet{
             CondRet ret{true, true};
             if(closed){
               //通道关闭，自然不能再排队数据
               ret.can_queue = false;
               return ret;
             }

             //写入时，若等待队列中的元素个数小于容量，则不需要等待 ---> 直接写入
             if(size < m_capacity){
               ret.need_wait = false;
               return ret;
             }
             //到这说明容量已满，接下来的写入需要阻塞等待，但若时间允许，则也不能入队
             if(!time_wait){
               ret.can_queue = false;
               return ret;
             }
             return ret;
        };

        Status status;
        if(deadline == TimePoint())
          status = write_cond.wait(lock,entry,cond);   //将数据写入等待队列中并等待
        else 
          status = write_cond.wait_until(lock,deadline,entry,cond);  
        switch ((int)status){
          case (int)Status::NO_TIMEOUT:
            if(closed)
              return false;
            return true;

            //超时。写入失败
          case (int)Status::TIMEOUT:
            wait -= one_write;
            return false;

            //没有入队
          case Status::NO_QUEUED:
            //debug调试
            wait -= one_write;
            return false;
          default:
            break;
        }
        return false;
      }


      bool pop(T & t, bool time_wait, FastSteadyClock::time_point deadline = FastSteadyClock::time_point{}){

        int spin = 0;
        uint64_t wait;

        for(;;){
          wait = m_wait.load(std::memory_order_relaxed);
          //含有等待的写者
          if(wait & write_mask){
            if(!write_cond.notify_one(
                  [&](Entry& entry){ //唤醒写者时。读取数据
                    t = entry.val;
                  }
                  )){
              //自旋超时
              if(++spin >= spin_count){
                  spin = 0;
                  Processer::FiberYield();
              }
              //每次自旋时的检查
              if(m_capacity == 0){
                if(closed)
                  return false;
                else if(!time_wait)
                  return false;
              }
              continue;

            }
            wait -= one_write;
            return true;
          }else if(m_capacity == 0){
                if(closed)
                  return false;
                else if(!time_wait)
                  return false;
          }
         if(m_wait.compare_exchange_weak(wait,wait+one_read,std::memory_order_acq_rel,std::memory_order_relaxed))
          break;
        }
        FakeLock lock;
        Entry entry;
        //entry.id 
        entry.p_val = &t;
        auto cond = [&](size_t size)->CondRet{
             CondRet ret{true, true};
             if(closed){
               //通道关闭，自然不能再读出数据
               ret.can_queue = false;
               return ret;
             }
        };

        Status status;
        if(deadline == TimePoint())
          status = read_cond.wait(lock,entry,cond);   //将数据写入等待队列中并等待
        else 
          status = read_cond.wait_until(lock,deadline,entry,cond);  
        switch ((int)status){
          case (int)Status::NO_TIMEOUT:
            if(closed)
              return false;
            return true;
            //超时。读取失败
          case (int)Status::TIMEOUT:
            wait -= one_read;
            return false;

          case Status::NO_QUEUED:
            //debug调试信息
            wait -= one_read;
            return false;
          default:
            break;
        }
        return false;
      }
      //以写者队列为依据
    std::size_t size(){
        return std::min<size_t>(m_capacity,write_cond .size());
    }
    void Close(){
        if (closed)
          return ;
        closed = true;
        read_cond.notify_all();
        write_cond.notify_all();
    }



   };
}
