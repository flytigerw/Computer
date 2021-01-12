
#ifndef MY_THREAD_H
#define MY_THREAD_H



#include <pthread.h>
#include <functional>
#include <memory>
#include <thread>
#include <semaphore.h>
#include <stdint.h>

namespace tiger{

  //核心:在构建时调用lock,在析构时进行unlock
  //同时提供手动的lock+unlock ---> 增加灵活性 ----> 但需要用变量表示lock的状态
  //为什么使用模板? ----> 可能由多种Mutex具有相同的调用API
  template<class T>
  class MutexLockGuard{
    public:
      MutexLockGuard(T& mutex):m_mutex(mutex){
          m_mutex.lock();
          is_locked = true;
      }
      ~MutexLockGuard(){
          m_mutex.unlock();
      }
      void lock(){
        if(!is_locked){
          m_mutex.lock();
          is_locked = true;
        }
      }
      void unlock(){
        if(is_locked){
          m_mutex.unlock();
          is_locked = false;
        }
      }

    private:
      bool is_locked;
      T& m_mutex;
  };

  class Mutex{
    public:
      //方便使用
      typedef MutexLockGuard<Mutex> LockGuard;
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

  //自旋锁，得到就空转 ---> 不陷入内核态 ----> 使用于冲突多而短
  class SpinMutex{
    public:
      typedef MutexLockGuard<SpinMutex> LockGuard;
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
  class RWMutex {
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


  class Semaphore{
    public:
      Semaphore(uint32_t n = 0){
        if(sem_init(&m_semaphore,0,n))
          throw std::logic_error("sem_init error");
      }
      ~Semaphore(){
         std::logic_error("sem_destroy error");
      }
      //+1
      void notify(){
        if(sem_post(&m_semaphore))
          throw std::logic_error("sem_post error");
      }
      //-1
      void wait(){
        if(sem_wait(&m_semaphore))
          throw std::logic_error("sem_wait error");
      }
    private:
      sem_t m_semaphore;
  };
 
  
  class Thread{
    public:
      typedef std::shared_ptr<Thread> ptr;
      Thread(const std::string& name,std::function<void()>m_task);
      ~Thread();
      void join();
      pid_t get_os_id() const {return os_id;}
      const std::string& get_name() const{return m_name;}

      //static函数:定义在类中，便于通过 类名:: 取出使用 -->且也是类的友元
      //线程从此开始执行
      static void* run(void*);
      
      static Thread* GetCurThread();
      static const std::string& GetCurThreadName();

    private:
      //os内核层面的线程ID ---> unique
      pid_t os_id;
      //POSIX线程库的线程ID ----> 不同进程之间可能重复
      pthread_t p_id = 0;  //0也可以作为状态位
      std::string m_name;
      std::function<void()> m_task;

      //用于Thread创建和运行两个动作的同步
      Semaphore m_semaphore;
  };


}
#endif
