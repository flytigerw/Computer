#ifndef  THREAD_H
#define  THREAD_H 


#include <pthread.h>
#include <functional>
#include <memory>
#include <thread>
#include <semaphore.h>
#include <stdint.h>
#include "noncopyable.h"

namespace sylar{
  
  //mutex可能有多种，将其设计为模板类
  template<class T>
    class LockGuardImpl{
      public:
        LockGuardImpl(T& mutex) : m_mutex(mutex){
          //在构造函数中lock
          m_mutex.lock();
          m_locked = true;
        }
        ~LockGuardImpl(){
          //在析构函数中unlock
          unlock();
        }

        //解锁与加锁时都要考虑是否已经有锁 ---> 避免自身线程重复加锁
        void lock(){
          if(!m_locked){
            m_mutex.lock();
            m_locked = true;
          }
        }

        void unlock(){
          if(m_locked){
            m_mutex.unlock();
            m_locked = false;
          }
        }

      private:
        T& m_mutex;
        bool m_locked;
    };

  class Mutex : public Noncopyable{
    public:
      typedef LockGuardImpl<Mutex> LockGuard;
      Mutex(){
        pthread_mutex_init(&m_mutex,nullptr);
      }
      ~Mutex(){
        pthread_mutex_destroy(&m_mutex);
      }
      void lock(){
        pthread_mutex_lock(&m_mutex);
      }

      void unlock(){
        pthread_mutex_unlock(&m_mutex);
      }
    private:
      pthread_mutex_t m_mutex;
  };
  //用于验证线程安全 ----> 此Mutex has nothing ----> 与mutex形成对比
  class NullMutex{
    public:
      typedef LockGuardImpl<NullMutex> LockGuard;
      NullMutex(){}
      ~NullMutex(){}
      void lock(){}
      void unlock(){}
  };

  template<class T>
    class ReadLockGuardImpl{
      public:
        ReadLockGuardImpl(T& mutex) : m_mutex(mutex){
          m_mutex.read_lock();
          m_locked = true;
        }
        ~ReadLockGuardImpl(){
          unlock();
        }
        void lock(){
          if(!m_locked){
            m_mutex.read_lock();
            m_locked = true;
          }
        }
        void unlock(){
          if(m_locked){
            m_mutex.unlock();
            m_locked = false;
          }
        }

      private:
        T& m_mutex;
        bool m_locked;
    };
  template<class T>
    class WriteLockGuardImpl{
      public:
        WriteLockGuardImpl(T& mutex) : m_mutex(mutex){
          m_mutex.write_lock();
          m_locked = true;
        }
        ~WriteLockGuardImpl(){
          unlock();
        }
        void lock(){
          if(!m_locked){
            m_mutex.write_lock();
            m_locked = true;
          }
        }
        void unlock(){
          if(m_locked){
            m_mutex.unlock();
            m_locked = false;
          }
        }
      private:
        T& m_mutex;
        bool m_locked;
    };



  //多个读者可并发的持有锁 -----> 记录读者的数量
  //读者和写者，写者和写者都是互斥的持有锁 -----> 记录哪一类人在持有锁
  class RWMutex : public Noncopyable{

    public:
    typedef ReadLockGuardImpl<RWMutex> ReadLockGuard;
    typedef WriteLockGuardImpl<RWMutex> WriteLockGuard;
      RWMutex(){
        pthread_rwlock_init(&m_lock,nullptr);
      }
      ~RWMutex(){
        pthread_rwlock_destroy(&m_lock);
      }
      //读者加锁
      void read_lock(){
        pthread_rwlock_rdlock(&m_lock);
      }
      //写者加锁
      void write_lock(){
        pthread_rwlock_wrlock(&m_lock);
      }
      void unlock(){
        pthread_rwlock_unlock(&m_lock);
      }
    private:
      pthread_rwlock_t m_lock;
  };

  class NullRWMutex{
    public:
     typedef ReadLockGuardImpl<NullRWMutex> ReadLockGuard;
     typedef WriteLockGuardImpl<NullRWMutex> WriteLockGuard;
      NullRWMutex(){}
      ~NullRWMutex(){}
      void read_lock(){}
      void write_lock(){}
      void unlock(){}
  };

  //自旋锁，得到就空转 ---> 不陷入内核态 ----> 使用于冲突多而短
  class SpinMutex{

    public:
      typedef LockGuardImpl<SpinMutex> LockGuard;
      SpinMutex(){
        pthread_spin_init(&m_mutex,0);
      }
      ~SpinMutex(){
        pthread_spin_destroy(&m_mutex);
      }
      void lock(){
        pthread_spin_lock(&m_mutex);
      }
      void unlock(){
        pthread_spin_unlock(&m_mutex);
      }
    private:
      pthread_spinlock_t m_mutex;
  };
  

  class Semaphore : public Noncopyable {
    public:
      //初值
      Semaphore(uint32_t count=0);
      ~Semaphore();
      //+1操作
      void notify();
      //-1操作
      void wait();
    private:
      sem_t m_semaphore;
  };
  class Thread : public Noncopyable{

    public:
      typedef  std::shared_ptr<Thread> ptr;
      Thread(std::function<void()> cb,const std::string& name) ;
      ~Thread();

      const std::string& getName() const{return m_name;}
      pid_t getId() const {return m_id;}

      void join();
      static Thread* GetThis();          //------------------
      //用于在日志中获取线程名称
      static const std::string& GetName();
      static void setName(const std::string& name);
      //线程启动时的函数
      static void* run(void*);
    private:
      //禁止拷贝
    private:
      //线程元素:线程在OS层面的id,用户层面的id,执行函数,名称标识
      pid_t m_id;
      pthread_t m_thread = 0;
      //真正运行的函数
      std::function<void()> m_cb;
      std::string m_name;
      Semaphore m_semaphore;

  };
}

#endif
