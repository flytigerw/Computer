

#pragma once

#include "fiber_pool.h"

namespace tiger{


  
  FiberPool* FiberPool::Create(size_t max_cb_points){

    return new FiberPool(max_cb_points);  //返回裸指针...
  }

  FiberPool::FiberPool(size_t max_cb_points){
    
    m_max_count = 0;
    m_max_cb_points = max_cb_points;
    //先建一个scheduler
    m_scheduler = Scheduler::Create();

    //数组
    //大小:max_cb_points 
    //元素:CallBackPoint*
    cb_points = new CallBackPoint*[max_cb_points];


  }
  //将channel中回调函数，取出运行
  //max_trigger:最大回调数量
  size_t FiberPool::CallBackPoint::run(size_t max_trigger){
    size_t i=0;
    for(;i<max_trigger || max_trigger == 0; ++i){
      Func cb;
      if(!m_channel.try_pop(cb))
        break;
      cb();
    }
    return i;
  }

  void FiberPool::CallBackPoint::push(Func&& cb){
    m_channel << std::move(cb);
  }


  //设置唤醒函数
  void FiberPool::CallBackPoint::set_notify_func(const Func& notify){
    notify_func = notify;
  }

  //唤醒
  void FiberPool::CallBackPoint::notify(){
    if(notify_func)
      notify_func();
  }

  void FiberPool::go(){
    for(;;){

      PoolTask task;
      //取出一个task
      m_tasks >> task;
      if(task.m_func)   //执行任务
        task.m_func();

      if(!task.m_cb)
        continue;

      size_t count = points_count;
      if(!count){       //回调点为0 ---> 直接进行回调
        task.m_cb();  
        continue;
      }

      //含有回调点 ---> 将回调函数压入回调点.并notify
      size_t idx = ++robin & points_count;  //占用第几个回调点 ---> 取余循环
      cb_points[idx]->push(std::move(task.m_cb));
      cb_points[idx]->notify();
    }
  }

  void FiberPool::push(const Func& func,const Func& cb){
    PoolTask task{func, cb};
    m_tasks << std::move(task);
  }

  //没搞懂
  bool FiberPool::add_cb_point(FiberPool::CallBackPoint* point){
    //写入点+1
    size_t write_idx = write_points_count++;
    if(write_idx >= m_max_cb_points){  //为什么不直接判断write_points_count == m_max_count
      --write_points_count;
      return false;
    }

    cb_points[write_idx] = point;
    while (!points_count.compare_exchange_weak(write_idx, write_idx + 1,
                std::memory_order_acq_rel, std::memory_order_relaxed)) ;
    return true;
  }

  void FiberPool::init(size_t max_count){
    m_max_count = max_count;
  }

  //参数传给scheduler
  void FiberPool::Start(int min_thead_num,int max_thread_num){

    if(!lock.try_lock())  //只能调用一次
      return;
    //开辟线程运行scheduler
    std::thread([=]{
        m_scheduler->start(min_thead_num,max_thread_num);
        }).detach();

    if(m_max_count == 0){
      m_max_count = (std::max)(min_thead_num * 12,max_thread_num);
      //最多10240个
      m_max_count = (std::min<size_t>)(m_max_count, 10240);
    }

    for(size_t i=0;i<m_max_count;++i){
      
    }

  }
  


}
