
#pragma once

#include "../../scheduler/scheduler.h"
#include "../../channel/channel.h"


namespace tiger{

// 协程池
// 可以无缝与异步代码结合, 处理异步框架中的阻塞事件

  class FiberPool{

    public:
      typedef std::function<void()> Func;


      //回调point ---> 回调函数集
      class CallBackPoint{

        private:
          Channel<Func> m_channel;
          Func notify_func;

        private:
          friend class FiberPool;
          //新增一个回调函数
          void push(Func && cb);
          void notify();

        public:
          size_t run(size_t max_trigger = 0);
          void set_notify_func(const Func& notify);
      };
    private:
      struct  PoolTask{
        Func m_func,m_cb;
      };
    private:
      size_t m_max_count;  //最大协程数量
      std::atomic<int> m_count;
      //所有归属的调度器
      Scheduler* m_scheduler;

      Channel<PoolTask> m_tasks;

      size_t m_max_cb_points;

      std::atomic<size_t> points_count{0};
      std::atomic<size_t> write_points_count{0};
      std::atomic<size_t> robin{0};

      //二维数组
      CallBackPoint** cb_points;
      LFLock lock;


    private:
      //构造和析构函数私有化
      //通过static Create进行创建
      FiberPool(size_t max_cb_points);  //回调点
      ~FiberPool(){}

    private:
      void go();

    public:
       static FiberPool* Create(size_t max_cb_points = 128);
       //初始化协程池
       void init(size_t max_count);

       void start(int min_thead_num,int max_thread_num);

       void push(const Func& func,const Func& cb);

       template<class R>
         void push(const Channel<R>& r,const std::function<R()> func){
           push([=]{r << func();},NULL);  //将func的运行结果push到r中
         }
       template<class R>
         void push(const std::function<R()>& func,
                  const std::function<void(R&)> cb){
           std::shared_ptr<R> ctx(new R);
           push([=]{*ctx << func();},     //将函数运行的结果存放到ctx中
                [=]{cb(*ctx);});          //将ctx作为参数，调用cb
         }
 //     void push(const Channel<void>& r,const std::function<void()> func){
   //     push([=]{func();r << NULL;},NULL);  //将func的运行结果push到r中
    //   }


    // 绑定回调函数触发点, 可以绑定多个触发点, 轮流使用.
    // 如果不绑定触发点, 则回调函数直接在协程池的工作线程中触发.
    // 线程安全接口
    bool add_cb_point(CallBackPoint* point);





  };


}
