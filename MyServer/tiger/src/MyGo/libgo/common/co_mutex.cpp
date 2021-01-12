
#pragma  once 

#include "co_mutex.h"

namespace tiger{


  //表示资源可用
  CoMutex::CoMutex():m_count(1){}
  CoMutex::~CoMutex(){}

  void CoMutex::lock(){
    //上锁成功
    if(--m_count == 0)
      return;
    //m_count已经为0 ---> 阻塞在条件变量上
    std::unique_lock<LockType> lock_guard(m_lock);
    if(notified){
      notified = false;
      return;
    }
    m_cond_var.wait(lock);
  }

  bool CoMutex::try_lock(){
    if(--m_count == 0)
      return true;
    //上锁失败
    ++m_count;
    return false;
  }

  bool CoMutex::is_lock(){
    return m_count < 1;
  }

  void CoMutex::unlock(){
    //从0 ----> 1
    if (++m_count == 1)
        return ;
    //唤醒一个阻塞在条件变量上的
    std::unique_lock<LockType> lock_guard(m_lock);
    if (!m_cond_var.notify_one())
        notified = true;
  }
}
