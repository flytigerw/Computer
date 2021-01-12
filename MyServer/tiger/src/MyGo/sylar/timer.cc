#include "timer.h"
#include "util.h"

namespace  sylar{
  Timer::Timer(uint64_t ms,std::function<void()> cb,bool recurring,TimerManager* m):
              m_ms(ms),m_recurring(recurring),m_manager(m),m_cb(cb){
                //执行时间 = 起点+时间间隔
                m_next = GetCurrentMS() + m_ms;
  }

  bool Timer::Comparator::operator()(const Timer::ptr& lhs,const Timer::ptr& rhs) const{
    if(!lhs && !rhs)
      return false;
    if(!lhs)
      return true;
    if(!rhs)
      return false;
    //两个timer都存在，比较其执行时间
    if(lhs->m_next < rhs->m_next)
      return true;
    if(rhs->m_next < lhs->m_next)
      return false;
    //若执行时间相等，则比较地址
    return lhs.get() < rhs.get();

  }

  //将其从TimeManager中删除
  bool Timer::cancel(){
    TimerManager::RWMutexType::WriteLockGuard lock_guard(m_manager->m_mutex);
    if(m_cb){
      m_cb = nullptr;
      m_manager->m_timers.erase(shared_from_this());
      return true;
    }
    return false;
  }

  //from_now表示是否强制立马修改时间
  bool Timer::reset(uint64_t ms,bool from_now){
    if(ms == m_ms && !from_now )
      return  false;
    TimerManager::RWMutexType::WriteLockGuard lock_guard(m_manager->m_mutex);
    if(!m_cb)
      return false;   
    auto it = m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end())
      return false;
    m_manager->m_timers.erase(it);
    uint64_t start = 0;
    if(from_now)
      start = GetCurrentMS();
    else 
      start = m_next - m_ms;
    m_ms = ms;
    m_next = start+ m_ms;
    m_manager->addTimer(shared_from_this(),lock_guard);
    return true;
  }
  bool Timer::refresh(){
    TimerManager::RWMutexType::WriteLockGuard lock_guard(m_manager->m_mutex);
    if(!m_cb)
      return false;   //已经被cancel 
    auto it = m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end())
      return false;
    //set的key不能修改,只能先erase在填回去
    m_manager->m_timers.erase(it);
    //刷新执行时间
    m_next = sylar::GetCurrentMS() + m_ms;
    m_manager->m_timers.insert(shared_from_this());
    return true;
  }

  TimerManager::TimerManager(){
    previousTime = GetCurrentMS();
  }
  bool TimerManager::detectClockChange(uint64_t now_ms){
    bool changed = false;
    //当前时间比上次时间小 ---> 时间回调过
    if(now_ms < previousTime && now_ms <previousTime-60*60*1000)
      changed = true;
    previousTime  = now_ms;
    return changed;
  }
  Timer::ptr TimerManager::addTimer(uint64_t ms,std::function<void()>cb,bool recurring) {
    Timer::ptr new_timer(new Timer(ms,cb,recurring,this));

    RWMutexType::WriteLockGuard lock_guard(m_mutex);
    addTimer(new_timer,lock_guard);
    return new_timer;  //返回timer,可以取消
  }
  void TimerManager::addTimer(Timer::ptr val,RWMutexType::WriteLockGuard& lock_guard){
    //加入到m_timers中
    auto it = m_timers.insert(val).first;
    //添加m_notify避免多次修改
    bool at_front = (it == m_timers.begin() && !m_notify);
    if(at_front)
      m_notify = true;
    lock_guard.unlock();
    //有必要去唤醒
    if(at_front)
       onTimerInsertAtFront();
  }

  static void OnTimer(std::weak_ptr<void> weak_cond,std::function<void()> cb){
    std::shared_ptr<void> it=  weak_cond.lock();  //查看指针是否还有指向
    if(it)
      cb();
  }
  Timer::ptr TimerManager::addConditionTimer(uint64_t ms,std::function<void()> cb,std::weak_ptr<void> weak_cond,bool recurring){
    return addTimer(ms,std::bind(&OnTimer,weak_cond,cb),recurring);
  }


  //调用此方法重新设置epoll_wait
  uint64_t TimerManager::getNextTimer(){
    RWMutexType::ReadLockGuard lock_guard(m_mutex);
    m_notify = false;
    if(m_timers.empty())
      return ~0ull;
    //定时器的执行时间与当前时间的比较
    const Timer::ptr& next = *m_timers.begin();
    auto cur_ms = GetCurrentMS();
    if(cur_ms >= next->m_next){  //定时器晚了，没有执行
      return 0;
    }else 
      return next->m_next - cur_ms;
  }

  bool TimerManager::hasTimer(){
    RWMutexType::ReadLockGuard lock_guard(m_mutex);
    return !m_timers.empty();
  }

  void TimerManager::listExpiredCb(std::vector<std::function<void()>>& cbs){
    uint64_t cur_ms = GetCurrentMS();
    std::vector<Timer::ptr> expired;
    {
      RWMutex::ReadLockGuard lock_guard(m_mutex);
      if(m_timers.empty())
        return ;
    }
      //遍历每个定时器，看有哪些超时了
      RWMutex::WriteLockGuard lock_guard(m_mutex);

      bool changed = detectClockChange(cur_ms);
      if(!changed && ((*m_timers.begin())->m_next > cur_ms)){  //没有超时
        return ;
      }
      Timer::ptr now_timer(new Timer(cur_ms));
      //若系统时间改变过，则设it为end ---> 会将所有timer清理掉 ---> 再重新设置
      auto it = changed ? m_timers.end() : m_timers.lower_bound(now_timer);
      while(it != m_timers.end() && (*it)->m_next == cur_ms)
        ++it;
      expired.insert(expired.begin(),m_timers.begin(),it);
      //将超时timer清理掉
      m_timers.erase(m_timers.begin(),it);
      cbs.reserve(expired.size());
      //得到超时timer的cb 
      for(auto& timer:expired){
        cbs.push_back(timer->m_cb);
        if(timer->m_recurring){  //是循环计时器,重置时间，又放回去
            timer->m_next = cur_ms + timer->m_ms;
            m_timers.insert(timer);
        }else 
          timer->m_cb = nullptr;
      }
  }

}
