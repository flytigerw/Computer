#pragma once 
#include "../datastructrue/incursivestruct.h"
#include <condition_variable>
#include <mutex>
#include <thread>
#include "../scheduler/processor.h"

namespace tiger{
//协程条件变量
//与std::condition_variable_any的区别在于析构时不能有正在等待的协程, 否则抛异常

  //T ---> bool
  template<class T>
    class CoConditionVar{
      public:
       typedef std::function<void(T &)> Functor;
       enum class Status{
         TIMEOUT,   //可能为条件变量设置定时器
         NO_TIMEOUT,
         NO_QUEUED
       };

      private:
       //兼容原生线程
       struct NativeThreadEntry{
        std::mutex mtx;
        std::condition_variable_any cv;
        LFLock notified_lock;
       };
        
       //Suspend协程的一些标志
       enum SuspendFlag{
         SUSPEND_BEGIN = 0x1,
         SUSPEND_END = 0x2,
         WAKEUP_BEGIN = 0x4,
         WAKEUP_END = 0x8
       };



       //条件变量含有一个等待队列,队列中的元素记录了协程的一些信息
       struct Entry : public WaitNode, public RefCounter{
             Processer::SuspendEntry m_suspend_entry;           //suspend_entry是processor挂起协程后，引用返回的
             NativeThreadEntry* m_native_thread_entry = nullptr;
             LFLock time_lock;
             //状态机处理
             std::atomic<int> m_suspend_flag{0};
             //Entry时notify和wait共享的 ---> 数据传递
             T value;   
             bool is_waiting = true;
             ~Entry(){
               if(m_native_thread_entry){
                 delete m_native_thread_entry;
                 m_native_thread_entry = nullptr;
               }
             }

             bool notify(const Functor& f){
               for(;;){
                
                 int flag = m_suspend_flag.load(std::memory_order_relaxed);
                //正在被挂起 --->等待挂起动作结束
                 if(flag & SuspendFlag::SUSPEND_BEGIN){
                    while ((m_suspend_flag.load(std::memory_order_acquire) & SuspendFlag::SUSPEND_END) == 0);
                    break;
                 }
                 //没有挂起，直接唤醒
                 if(m_suspend_flag.compare_exchange_weak(flag,flag|SuspendFlag::WAKEUP_BEGIN,
                                                         std::memory_order_acq_rel,
                                                         std::memory_order_relaxed)){
                    flag |= (SuspendFlag::WAKEUP_BEGIN | SuspendFlag::WAKEUP_END);
                    
                    //没有获得锁 ---> WEAKUP半途而废 --> return false，然后继续WEAKUP
                    if (!time_lock.try_lock()) {
                        m_suspend_flag.store(flag, std::memory_order_release);
                        return false;;

                    }
                    //成功获得time_lock,对值进行更新
                    if(f)
                      f(value);
                   //原子性的更新flag
                    m_suspend_flag.store(flag, std::memory_order_release);
                    return true;  //WAKUP成功
                 }
               }
               //到这,说明已经 挂起动作 已经结束
               if(!m_native_thread_entry){
                 if(!time_lock.try_lock())
                   return false;
                 if(Processer::WakeUp(m_suspend_entry),[&]{if(f)f(value);}){
                    return true;
                 }
                 return false;
               }
             //针对native thread
            std::unique_lock<std::mutex> thread_lock(m_native_thread_entry->mtx);
            if(!time_lock.try_lock())
              return false;
            if(f)
              f(value);
            m_native_thread_entry->cv.notify_one();
            return true;
           }
       };


       WaitQueue<Entry> waiting;
       //在wait后是否需要重新lock
       bool relock_after_wait = true;
       //在析构时lock
       template <typename LockType>
       struct AutoLock{
        LockType* m_lock;
        bool is_locked;

        AutoLock(LockType & lock, bool locked) : m_lock(&lock), is_locked(locked) {}
       
        ~AutoLock() { if (is_locked) m_lock->lock(); }
    };


       
      public:
       typedef typename WaitQueue<Entry>::ConditionRet ConditionRet;
       typedef std::function<ConditionRet(size_t)> CondFunctor;
      //WaitQueue构造时的参数:check_fctor,distance,pos_fctor
       explicit CoConditionVar(size_t nonblockingCapacity = 0,        
                               Functor convertToNonblockingFunctor = nullptr)   //将元素从阻塞态转为非阻塞态
                               : waiting(&IsValid, nonblockingCapacity,
                                    [=](Entry *entry){
                                        if (entry->notify(convertToNonblockingFunctor)) {
                                          entry->isWaiting = false;
                                          return true;
                                        }
                                       return false;
                                    }){}

       ~CoConditionVar(){}
       void set_relock_after_wait(bool b) {  relock_after_wait= b; }


       //wait系列
       template<class LockType>
         Status wait(LockType& lock,T value = T(),const CondFunctor& cond = nullptr){ //value为默认 ---> 不重要
          std::chrono::seconds* time = nullptr;
          return do_wait(lock, time, value, cond);
        }
       //等待含有时间限制
        template <typename LockType, typename TimeDuration>
          Status wait_for(LockType & lock,
               TimeDuration duration,
               T value = T(),const CondFunctor& cond = NULL){
           return do_wait(lock, &duration, value, cond);
        }
        template <typename LockType, typename TimePoint>
          Status wait_until(LockType & lock,
               TimePoint timepoint,
               T value = T(),const CondFunctor& cond = NULL){
           return do_wait(lock, &timepoint, value, cond);
        }
        template <typename LockType, typename TimeType>
          Status do_wait(LockType& lock,
                         TimeType* time,
                         T value = T(),
                         const CondFunctor& cond = nullptr){
            //将其加到等待队列中
            Entry* entry = new Entry;
            AutoReleasePtr<Entry> p_entry(entry);
            entry->value = value;

            auto ret = waiting.push(entry);
            lock.unlock();  //释放锁，其他人可获得锁，并修改数据条件状态 ----> 即此刻可能被切出去调用notify

            AutoLock<LockType> auto_lock(lock,relock_after_wait);
            //检查此时其是否被唤醒
            int spinA = 0;
            int spinB = 0;
            int flag = 0;
            for(;;){
              flag = entry->m_suspend_flag.load(std::memory_order_relaxed);
              if(flag & SuspendFlag::WAKEUP_BEGIN){
              //正在被唤醒,等待 唤醒动作结束 --> 结束以后也就用在设立阻塞标志了
                while((entry->m_suspend_flag.load(std::memory_order_acquire) & SuspendFlag::WAKEUP_END) == 0);
                return Status::NO_TIMEOUT;
              }else{
                //并没有正在被唤醒 先自旋等一等再真正挂起
                if (++spinA <= 1)   //自旋一个++时间
                    continue;
                
                //处于协程环境中 ---> 切换到当前processor的主协程中
                if(Processer::IsCoroutine()){
                  if(++spinB <= 1){
                    Processer::FiberYield();
                    continue;
                  }
                }else{ //->线程切换
                  if(++spinB <= 8){  //切换次数不一样
                    std::this_thread::yield();
                    continue;
                  }
                }
              }
              //状态修改
              //////SUSPEND_BEGIN
            if (entry->suspendFlags.compare_exchange_weak(flag,flag | SuspendFlag::SUSPEND_BEGIN,
                                                          std::memory_order_acq_rel,
                                                          std::memory_order_relaxed))
              break;
            }

          flag |= (SuspendFlag::SUSPEND_BEGIN | SuspendFlag::SUSPEND_END);
          if(Processer::IsCoroutine()){
            //协程挂起
            fiber_suspend(entry->m_suspend_entry,time);
            //状态写回.
            /////SUSPEND_END
            entry->suspendFlags.store(flag, std::memory_order_release);   // release
            //再次切换
            Processer::FiberYield();
            return entry->time_lock.try_lock() ?  //获得了时间锁 ---> TIMEOUT
                   Status::TIMEOUT :
                   Status::NO_TIMEOUT;
          }else{
            entry->m_native_thread_entry = new NativeThreadEntry;
            std::unique_lock<std::mutex> thread_lock(entry->m_native_thread_entry->mtx);
            entry->suspendFlags.store(flag, std::memory_order_release);   // release
            //线程挂起
            thread_suspend(entry->m_native_thread_entry->cv,thread_lock,time);
            return entry->time_lock.try_lock() ?  //获得了时间锁 ---> TIMEOUT ---> 什么时候会获得失败呢 -->notify已经占有了锁
                   Status::TIMEOUT :
                   Status::NO_TIMEOUT;
          }

  }
        //引用返回entry
        //协程挂起
       template <typename TimeType>
         inline void fiber_suspend(Processer::SuspendEntry & entry, TimeType * time){
           if(time)
             entry = Processer::Suspend(*time);
           else 
             entry = Processer::Suspend();
         }
        

       //线程挂起 : 阻塞在条件变量上
       template <typename LockType, typename Rep, typename Period>
        inline void thread_suspend(std::condition_variable_any & cv,
            LockType & lock, std::chrono::duration<Rep, Period> * dur){
           if (dur)
              cv.wait_for(lock, *dur);
           else
              cv.wait(lock);
       }

      template <typename LockType, typename Clock, typename Duration>
        inline void threadSuspend(std::condition_variable_any & cv,
            LockType & lock, std::chrono::time_point<Clock, Duration> * tp){
           if (tp)
              cv.wait_until(lock, *tp);
           else
              cv.wait(lock);
        }
     

      //notify系列 ---------------------
      
      bool notify_one(const Functor& f=nullptr){
        //从等待队列中删除队首
        Entry* entry = nullptr;
        while(waiting.pop(entry)){
            AutoReleasePtr<Entry> pEntry(entry);
              //没有处于等待状态
              if(!entry->is_waiting){
                if(f)
                  f(entry->value);
                return true;
              }
              if(entry->notify(f)) //可能调用失败
                return true;
        }
        return false;
      }
      //返回唤醒的个数
      size_t notify_all(const Functor& f= nullptr){
        size_t n = 0;
        while(notify_one(f))
          ++n;
        return n;
      }
      
      bool is_empty(){
        return waiting.is_empty();
      }
      bool get_size(){
        return waiting.get_size();
      }
       //检验该entry是否有效
      static bool IsValid(Entry* entry){
        //位于等待对列中
        if (!entry->is_waiting) 
          return true;
        //已经被挂起
        if ((entry->m_suspend_flag & SuspendFlag::SUSPEND_BEGIN) == 0)
          return true;
        if (!entry->m_native_thread_entry)
            return !entry->m_suspend_entry.IsExpire();
        return entry->m_native_thread_entry->notified.is_lock();
    }

    };

}
