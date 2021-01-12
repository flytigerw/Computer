
#pragma once

#include <atomic>

namespace tiger{
   //基于CAS的实现的无锁 spinlock 
  struct LFLock{
    
    std::atomic_flag flag;
    //原子性地检测该标志:若不符合预期，则再度尝试 ---> 自旋行为
    void lock(){
      while(flag.test_and_set(std::memory_order_acquire));
    }

    void unlock(){
        flag.clear(std::memory_order_release);
    }

    bool try_lock(){
       return !flag.test_and_set(std::memory_order_acquire);

    }
  };

 struct FakeLock {
    void lock() {}
    bool is_lock() { return false; }
    bool try_lock() { return true; }
    void unlock() {}
 };
}

