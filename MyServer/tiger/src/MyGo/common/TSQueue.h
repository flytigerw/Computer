



#pragma once


#include <list>
#include <mutex>
#include "../schduler/fiber.h"
#include "lock.h"

namespace tiger{


  class TSQueue{

    public:

      typedef std::unique_lock<LFLock> LockGuard;

      void push(Fiber::ptr&& t){
        LockGuard lock_guard(m_lock);
        m_list.push_back(std::move(t));
      }
      void push(std::list<Fiber::ptr>&& list){
        LockGuard lock_guard(m_lock);
        m_list.splice(m_list.end(),std::move(list));
      }

      void front(Fiber::ptr& out) const {
        LockGuard lock_guard(m_lock);
        if(m_list.empty())
          out = nullptr;
        out = m_list.front();
      }
      
      size_t size() const {
        LockGuard lock_guard(m_lock);
        return m_list.size();
      }

      Fiber::ptr pop(){
        LockGuard lock_guard(m_lock);
        Fiber::ptr res = m_list.front();
        m_list.pop_front();
        return res;
      }

      std::list<Fiber::ptr> pop_front(uint32_t n){
        LockGuard lock_guard(m_lock);
        std::list<Fiber::ptr> res;
        auto it = m_list.begin();
        std::advance(it,n);
        res.splice(res.begin(),m_list,m_list.begin(),it);
        return res;
      }

      bool empty() const{
        LockGuard lock_guard(m_lock);
        return m_list.empty();
      }


    private:
      mutable LFLock m_lock;
      std::list<Fiber::ptr> m_list;
  };


}
