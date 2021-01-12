#pragma once 

#include <condition_variable>
#include "refsystem.h"
#include "clock.h"
#include "../datastructrue/incursivestruct.h"
#include "lock.h"



namespace tiger{



  template<class F>
    //Timer有自己的id
    class Timer: public IDCounter<Timer<F>>{

      struct TimerElement:public ListNode,
                           public RefCounter,
                           public IDCounter<TimerElement>{
          //一个Timer所包含的元素:回调函数,timeout时间点,用于保护的锁.这里采用时间槽，故需要知道自己位于哪一个槽中
          F m_cb;
          LFLock lock;
          FastSteadyClock::time_point m_tp;
          void* volatile m_slot = nullptr;

          TimerElement(const F& cb,FastSteadyClock::time_point tp):m_cb(cb),m_tp(tp){
          }

          //回调
          void call() noexcept{
            std::unique_lock<LFLock> lock_guard(lock,std::defer_lock);
            if(!lock_guard.try_lock()) //保证回调只被调用一次
              return;
            m_cb();
          }
          //取消定时器:从时间槽中删除
          bool cancel(){
            std::unique_lock<LFLock> lock_guard(lock,std::defer_lock);
            if(!lock.try_lock())
              return false;
            if(m_slot)
             ((TSQueue<TimerElement>*)m_slot)->erase(this);
            this->decrement();
            return true;
          }
      };
      
      //时间槽
      typedef TSQueue<TimerElement> TimerSlot;
      //时间池
      typedef TSQueue<TimerElement> TimerPool;

      public:
      //封装IncursivePtr<TimerElement>:加点功能
      class TimerPtr{
        friend class Timer;
        public:
           TimerPtr();
           explicit TimerPtr(TimerElement* p) : ptr(p){}
           explicit operator bool() const{
             return !!ptr;
           }
           bool is_stop_timer(){
             bool successed = true;
             if(ptr)
               successed = ptr->cancel();
             ptr.reset();
             return successed;
           }
        private:
          IncursivePtr<TimerElement> ptr;
      };

      public:
      volatile bool is_stop = false;
    
      //结束时？
      std::mutex quit_mtx;
      std::condition_variable_any m_quit;   

      TimerPool m_pool;
      int max_pool_size = 0;
      //八种level,每种level有256根链表
      TimerSlot m_slots[8][256];

      FastSteadyClock::time_point m_start_time;
      FastSteadyClock::duration  m_precision;
      
      //如何量化现在的时间?
      //(cur-start) / precision ---> rep
      union TimePoint{   
        uint64_t rep;   
        uint8_t idx[8];
      };
      TimePoint last_tp;
      //slots上的定时器已经timeout,需要立刻执行
      TimerSlot timeout_slots;
      
      public:
       Timer(){
        m_start_time = FastSteadyClock::now();
        m_precision = std::chrono::microseconds(100);
      }

       //精度设置
       template<class Rep,class Period>
         void set_precision(std::chrono::duration<Rep,Period> precision){
            //精度范围 [100ms,1min]
        if (precision < std::chrono::microseconds(100))
            precision = std::chrono::microseconds(100);
        if (precision > std::chrono::minutes(1))
            precision = std::chrono::minutes(1);
        //也就确定了底层的FastSteadyClock的精度
        m_precision = std::chrono::duration_cast<FastSteadyClock::duration>(precision);

         }


       void set_pool_size(int max,int reserve=0);

       size_t get_pool_size(){
         return m_pool.size();
       }

       //创建一个定时器并启动
       TimerPtr create_timer(FastSteadyClock::duration dur,const F&  cb){
         return create_timer(FastSteadyClock::now()+dur,cb);

       }
       TimerPtr create_timer(FastSteadyClock::time_point tp,const F&  cb){
         TimerElement* timer = new TimerElement(tp,cb);
         TimerPtr p(timer);
         insert_to_slots(timer,false);
         return p;
       }

       void thread_run(){
         while(!is_stop){
           run_once();
           std::this_thread::sleep_for(m_precision);
         }
        std::unique_lock<std::mutex> lock_guard(quit_mtx);
        m_quit.notify_one();
       }
       //设置结束标志位
       //要等thread_run运行结束后才能stop
       void stop(){
         is_stop = true;
        std::unique_lock<std::mutex> lock_guard(quit_mtx);
        m_quit.wait(lock_guard);
       }

       //线程运行Timer时,每个周期需要做的事
       void run_once();
       //最近的下一次触发是什么时候
       FastSteadyClock::time_point next_trigger(FastSteadyClock::duration max);

      private:
        //将timer插入时间槽中
        //@mainloop: 是否在触发线程中, 如果为true, 则无需检验是否需要加入completeSlot_.
        void insert_to_slots(TimerElement* t,bool main_loop);

        //执行slot中的timer
        void dispatch(TimerSlot& slot, FastSteadyClock::time_point now);


        //类似dispatch，但不需要进行触发判断
        void trigger(TimerSlot & slot);

        //共外界使用
        static void DeleteTimer(RefCounter* ptr, void* arg);
        //内部实现
        void delete_timer(TimerElement*);


    };

template <typename F>
  void Timer<F>::run_once(){

      //触发已经timeout的timer
      trigger(timeout_slots);

      auto now = FastSteadyClock::now();
      //时间变化量
      uint64_t delta_from_start= std::chrono::duration_cast<FastSteadyClock::duration>
                                (now-m_start_time).count() / m_precision.count();
      
      TimePoint& cur_tp = *(TimePoint*)&delta_from_start;
      //一个周期内的时间变化
      TimePoint delta;
      delta.rep = cur_tp.rep - last_tp.rep;

      //更新last_tp,并使用副本完成接下来的操作
      auto last_cpy = last_tp;

      //原子性地更新last_tp
      __atomic_store(&last_tp.rep, &cur_tp.rep, std::memory_order_release);

      int trigger_level = 0;
      //每级扫过了几个slot
      int trigger_slots[8] = {};
      const uint64_t additions[8] = {(uint64_t)1, 
                                     (uint64_t)1 << 8,           //第2个字节 level2
                                     (uint64_t)1 << 16,
                                     (uint64_t)1 << 32,
                                     (uint64_t)1 << 40,
                                     (uint64_t)1 << 48, 
                                     (uint64_t)1 << 56, 
                                     (std::numeric_limits<uint64_t>::max)()};

      //需要执行的timer
      TimerSlot executes;
      TimePoint tp;

      uint64_t i =0;
      while(i < delta.rep){
        tp.rep = last_cpy.rep+i;
        int level = trigger_level;       //level相当于横坐标x
        int slot_idx = tp.idx[level];    //slot_idx相当于纵坐标y

        trigger(m_slots[level][slot_idx]);

        //一个level上含有256个格子，扫完以后才能进行下一个层
        if(++trigger_slots[level] == 256)
          ++trigger_level;      

        while(tp.rep > 0 && slot_idx == 0){    
          ++level;
          slot_idx = tp.idx[level];     //每一种level的slot
          executes.push(m_slots[level][slot_idx].pop_all());

          ++trigger_slots[level];
        }
         uint64_t add = additions[trigger_level];
         if(trigger_level > 0){
            uint64_t levelupSteps = (256 - tp.idx[trigger_level - 1]) * additions[trigger_level - 1];
            if (levelupSteps > 0)
                add = (std::min)(levelupSteps, add);
         }
         i += add;
      }
      dispatch(executes, now);
  }



template <typename F>
  void Timer<F>::insert_to_slots(TimerElement * element, bool mainloop){
sync_retry:
        TimePoint last_cpy;
        __atomic_load(&last_tp.rep,&last_cpy.rep,std::memory_order_acquire);
        FastSteadyClock::time_point last_time(m_start_time+last_cpy.rep*m_precision);
        //已经timeout
        if(!mainloop && element->m_tp <= last_time){
          timeout_slots.push(element);
          return;
        }
      uint64_t delta_from_last= std::chrono::duration_cast<FastSteadyClock::duration>
                                   (element->m_tp-last_time).count() / m_precision.count();

      TimePoint& tp = *(TimePoint*)&delta_from_last;
      int level = 0;
      int offset = last_cpy.idx[0];
      //从左到右找到第一个idx[level]>0
      for(int i=7;i>=0;--i){
        if(tp.idx[i] > 0){
          level = i;   //计时器归属的等级
          //只要低8位 --> 循环使用 --> 计时器归属的slot ---> 肯定排在last_cpy后面
          offset = (tp.idx[i] + last_cpy[i]) & 0xff;
          break;
        }
      }
      auto& slot = m_slots[level][offset];
      {
        std::unique_lock<typename TimerSlot::LockType> lock_guard(slot.lock_ref());
        uint64_t test;
		    __atomic_load(&last_tp.rep, &test, std::memory_order_acquire);
        if(last_cpy.rep != test)   //在run_once中，last_tp可能被修改
            goto sync_retry;  //lock_guard怎么办?
        
        //添加到相应的slot中
        slot.push_without_lock(element);
        element->m_slot = &slot;
      }
  }



template <typename F>
  void Timer<F>::dispatch(TimerSlot& slot, FastSteadyClock::time_point now){
      SList<TimerElement> slist = slot.pop_all();
      for(auto& i: slist){
        slist.erase(&i);
        //已经timeout
        if(i.m_tp <= now){
          i.call();
          i.decrement();
        }else{
          insert_to_slots(&i,true);
        }
      }
      slist.clear();
  }
template <typename F>
  void Timer<F>::trigger(TimerSlot& slot){
      SList<TimerElement> slist = slot.pop_all();
      for(auto& i: slist){
        slist.erase(&i);
          i.call();
          i.decrement();
      }
      slist.clear();
  }

template <typename F>
  FastSteadyClock::time_point Timer<F>::next_trigger(FastSteadyClock::duration hint_time){
    if (hint_time.count() == 0)
      return FastSteadyClock::now();
    if (!timeout_slots.empty()) 
      return FastSteadyClock::now();

    auto dest_time = FastSteadyClock::now() + hint_time;
    uint64_t delta_rep = std::chrono::duration_cast<FastSteadyClock::duration>(
            dest_time - m_start_time).count() / m_precision.count();
    TimePoint& tp = *(TimePoint*)&delta_rep;
    TimePoint last_cpy;
	  __atomic_load(&last_tp.p64, &last_cpy.p64, std::memory_order_acquire);

    auto last_time = last_cpy.rep*m_precision+m_start_time;
    if(last_cpy.rep >= tp.rep)
      return FastSteadyClock::now();

    //寻找此次需要检查的最大刻度
    int level = 0;
    for(int i=7; i>=0; --i){
      if(last_cpy.idx[i] < tp.idx[i]){
        level = i;
        break;
      }
    }
    //从小刻度到大刻度依次寻找timer
    for(int i=0;i<level;i++){
      for(int k=0;k<256;k++){
        //slot idx
        int j = (k+last_cpy.idx[i]) & 0xff;
        //找到timer
        if(!m_slots[i][j].empty()){
          tp.idx[i] = j;
          //高位清空
          for(int m=i+1;m<8;++m)
            tp.idx[m] = last_cpy.idx[m] = 0; 
          // 进位
          // 考虑到循环
           if (j < last_cpy.idx[i])
             tp.idx[i+1]  = 1;
           goto check_done;
        }
      }
    }

    //level下的范围:[]
    for(int j=last_cpy.idx[level];j<tp.idx[level];++j){
      if(!m_slots[level][j].empty()){
        tp.idx[level] = j;
        goto check_done;
      }
    }

check_done:
    if(tp.rep > last_cpy.rep)
      return (tp.rep-last_cpy.rep)*m_precision + last_time;

    return FastSteadyClock::now();
  }


template <typename F>
  void Timer<F>::DeleteTimer(RefCounter* ptr, void* arg){
        Timer* self = (Timer*)arg;
        self->delete_timer((TimerElement*)ptr);
  }

template <typename F>
  void Timer<F>::delete_timer(TimerElement* t){
    //如果timer pool还有空间，就放入pool中
    if((int)m_pool.size() < max_pool_size){
      //reset
        t->decrement();
        t->m_cb = F();
        m_pool.push(t);
    }else 
      delete t;
  }

template <typename F>
  void Timer<F>::set_pool_size(int max, int size){
        max_pool_size = max;
        if((int)m_pool.size() < size){
          //用一个临时非线程安全的队列
            TSQueue<TimerElement,false> pool;
            for(int i=m_pool.size();i<size;++i){
              auto p = new TimerElement;
              p->set_deleter(Deleter(&Timer<F>::DeleteTimer,(void*)this));
              pool.push(p);
            }
            m_pool.push(pool.pop_all());
        }

  }


}
