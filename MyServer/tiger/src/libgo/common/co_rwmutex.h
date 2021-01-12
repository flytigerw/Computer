

#pragma  once 

#include "../../base.h"
#include "lock.h"
#include "co_condition.h"

namespace tiger{


  class CoRWMutex{
    public:
      typedef CoConditionVar<bool> CondVar;
    
    private:


      LFLock m_lock;
      long lock_state;  //0:无锁 >=1:读锁 -1:写锁
      bool m_write_first;  //是否写优先

      CondVar read_cond,write_cond;

    public:

      explicit CoRWMutex(bool write_first = true);
      ~CoRWMutex();

      void read_lock();
      bool read_try_lock();
      void read_unlock();

      void write_lock();
      bool write_try_lock();
      void write_unlock();

      bool is_lock();


    public:
      //分为读写两个视角
      class ReadView : public Noncopyble{
        friend class CoRWMutex;
        
        private:
          CoRWMutex* self;

        public:
         void lock();
         bool try_lock();
         bool is_lock();
         void unlock();
      };

      class WriteView : public Noncopyble{
        friend class CoRWMutex;
        
        private:
          CoRWMutex* self;

        public:
         void lock();
         bool try_lock();
         bool is_lock();
         void unlock();
      };

      ReadView& reader();
      WriteView& writer();
      
    private:
      ReadView m_read_view;
      WriteView m_write_view;

      //无锁时，唤醒相关人员拿锁
      void try_wake_up();

  };
}
