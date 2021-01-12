

#include "co_rwmutex.h"
#include <mutex>


namespace tiger{

  CoRWMutex::CoRWMutex(bool write_first) : lock_state(0),m_write_first(write_first){
            m_read_view.self = this;
            m_write_view.self = this;
          }
  CoRWMutex::~CoRWMutex(){
            m_read_view.self = m_write_view.self = nullptr;

  }
  void CoRWMutex::read_lock(){
    std::unique_lock<LFLock> lock_guard(m_lock);

retry:

    if(m_write_first){  //写优先
      //没有写锁且没有写者在等待 ---> 若有写者在等待，就先写者优先获得锁
      if(lock_state >= 0 && write_cond.is_empty()){
        ++lock_state;
        return;
      }
    }else{  //读优先
      if(lock_state >= 0){  //只要不是写锁，就++lock_state ---> 表明读者+1
        ++lock_state;
        return;
      }
    }
    //到这说明含有写锁 --> 读者等待
    read_cond.wait(m_lock);
    //唤醒后，再次尝试拿锁
    goto retry;
  }

  bool CoRWMutex::read_try_lock(){
    std::unique_lock<LFLock> lock_guard(m_lock);
    if(lock_state >= 0){
      ++lock_state;
      return true;
    }
    //若有写锁，就直接返回
    return false;
  }

  void CoRWMutex::read_unlock(){
    std::unique_lock<LFLock> lock_guard(m_lock);
    if(--lock_state > 0)
      return;
    // =0 ---> 无锁状态 ---> 唤醒写者拿锁
    try_wake_up();
  }

  void CoRWMutex::write_lock(){
    std::unique_lock<LFLock> lock_guard(m_lock);
retry:
    if(lock_state == 0){  //只有处于无锁状态才可能拿到锁
      lock_state = -1;
      return;
    }
    //阻塞等待
    write_cond.wait(m_lock);
    goto retry;
  }

  bool CoRWMutex::write_try_lock(){
    std::unique_lock<LFLock> lock_guard(m_lock);
    if(lock_state == 0){
      lock_state = -1;
      return true;
    }
    return false;
  }

  void CoRWMutex::write_unlock(){
    std::unique_lock<LFLock> lock_guard(m_lock);
    lock_state = 0;
    try_wake_up();
  }
  void CoRWMutex::try_wake_up(){
    //优先唤醒写者
    if(write_cond.notify_one())
      return;
    read_cond.notify_all();
  }

  //是否有写锁
  bool CoRWMutex::is_lock(){
    std::unique_lock<LFLock> lock_guard(m_lock);
    return lock_state == -1;
  }

  CoRWMutex::ReadView& CoRWMutex::reader(){
    return m_read_view;
  }

  CoRWMutex::WriteView& CoRWMutex::writer(){
    return m_write_view;
  }

  void CoRWMutex::ReadView::lock(){
    self->read_lock();
  }
  bool CoRWMutex::ReadView::is_lock(){
    return self->is_lock();
  }
  void CoRWMutex::ReadView::unlock(){
    return self->read_unlock();
  }

  void CoRWMutex::WriteView::lock(){
    self->write_lock();
  }
  bool CoRWMutex::WriteView::try_lock(){
    return self->write_try_lock();
  }
  bool CoRWMutex::WriteView::is_lock(){
    return self->is_lock();
  }
  void CoRWMutex::WriteView::unlock(){
    self->write_lock();
  }

} 

