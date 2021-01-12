
#pragma once

#include <time.h>
#include <chrono>
#include <limits>
#include <mutex>
#include <thread>
#include "lock.h"

namespace tiger{


  //利用x86_64下的指令:rdtsc 
  //静态类 ---> 具有持久性和共享性
  class FastSteadyClock{
    public:
      typedef std::chrono::steady_clock clock;
      typedef clock::rep rep;
      typedef clock::period period;
      typedef clock::duration duration;
      typedef clock::time_point time_point;

    public:
      //应该专门开启一个来运行此clock 
      static void thread_run(){
        std::unique_lock<LFLock> lock_guard(Self().init_lock, std::defer_lock);

        //只能有一个线程来运行
        if(!lock_guard.try_lock())
          return;

        //以20ms为周期，计算该周期的delta_tsc / delta_t
        for(;;std::this_thread::sleep_for(std::chrono::microseconds(20))){
          auto& cur_point = Self().check_points[Self().turn];
          cur_point.m_tp = clock::now();
          cur_point.m_tsc = rdtsc();

          auto& last_point = Self().check_points[!Self().turn];
          if(last_point.m_tsc != 0){
              duration delta_t = cur_point.m_tp - last_point.m_tp;
              uint64_t delta_tsc = cur_point.m_tsc - cur_point.m_tsc;
              //分母不能为0
              float ratio = (float)delta_tsc / std::max<long>(delta_t.count(),1);
              //不能过小 ---> 可能时间精度比较大
              if (ratio < std::numeric_limits<float>::min())
                  ratio = std::numeric_limits<float>::min();
              Self().recycle_ratio = ratio;
              Self().init = true;
          }
          Self().turn = !Self().turn;

        }
        
      }

      static time_point now() noexcept{
        if(!Self().init)
          return clock::now();
        else{
          auto& last_point = Self().check_points[!Self().turn];
          uint64_t delta_tsc = rdtsc() - last_point.m_tsc;
          auto delta_t = delta_tsc/(Self().recycle_ratio);
          return time_point(duration((long)(last_point.m_tp.time_since_epoch().count()+delta_t)));
        }
      }



    private:
      struct Data{
        struct CheckTimePoint{
            time_point m_tp;
            uint64_t m_tsc;   
        };
        //只能有一个线程对其初始化
        LFLock init_lock;
        bool init = false;
        //一个周期内的delta_tsc / delta_t
        double recycle_ratio = 1;
        CheckTimePoint check_points[2];
        volatile int turn = 0;
      };
      //静态成员
      static Data& Self(){
        static Data obj;
        return obj;
      }

      //封装rdtsc指令
      static uint64_t rdtsc(){
        uint32_t high,low;
        __asm__ __volatile__(
            "rdtsc":"=a"(low),"=d"(high)
            );
        return ((uint64_t)high<<32) | low;
      }
  };

}
