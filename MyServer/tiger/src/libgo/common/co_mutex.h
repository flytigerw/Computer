

#pragma once 

#include "co_condition.h"

//协程锁

namespace tiger{

  class CoMutex{
   private:
    typedef std::mutex LockType;
    typedef CoConditionVar<bool> CondVar;

    LockType m_lock;
    CondVar  m_cond_var;
    bool notified = false;  //防止虚假唤醒
    std::atomic_long m_count;  //计数值

   public:
    CoMutex();
    ~CoMutex();

    void lock();
    bool try_lock();
    bool is_lock();
    void unlock();
  };


}
