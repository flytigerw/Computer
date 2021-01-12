

#pragma  once 


#include <deque>
#include <thread>
#include "../../thread.h"
#include "../common/lock.h"
#include "../common/timer.h"
#include "processor.h"



namespace  tiger{

  //创建fiber时的一些选项
  struct FiberOption{
    //亲和力
    bool afinity = false;
    //创建fiber时的源码位置
    const char* m_file = nullptr;
    int m_line = 0;

    size_t m_stack_size;
  };



  class Scheduler : public Noncopyble{
    friend class Processer;
    public:
      //静态实例
      static Scheduler& GetInstance();

      //创建一个Scheduler 
      static Scheduler* Create();

      //创建一个fiber 
      void create_fiber(const FuncType& fcn,const FiberOption& opt);
      
      //指明调度的processor范围
      //min_num: 最小调度线程数, 为0时, 设置为cpu核心数.
      //max_num: 最大调度线程数, 为0时, 设置为min_num.
      //如果max_num大于min_min, 则当协程产生长时间阻塞时,
      //可以自动扩展调度线程数.
      void start(int min_num = 1,int max_num = 0);

      void go_start(int min_num = 1,int max_num = 0);
      
      //停止调度 
      //注意: 停止后无法恢复, 仅用于安全退出main函数, 不保证终止所有线程.
      //如果某个调度线程被协程阻塞, 必须等待阻塞结束才能退出.
      void stop();


      uint64_t GetCurrentFiberId();
      //切换次数
      uint64_t GetCurrentFiberYieldCount();

      //fiber数量
      uint32_t fiber_count();

      bool is_stopped(){return stopped;}
      static bool& IsExiting();
      static void delete_fiber(RefCounter* tk, void* arg);

      typedef Timer<std::function<void()>> TimerType;
      //使用独立的定时器线程
      void use_alone_timer_thread();
      TimerType& static_timer();
      TimerType& get_timer(){
        return *m_timer ;
      }


    private:


    //直接维护Processor队列
    std::deque<Processer*> m_processers;

    //总的fiber数
    std::atomic<uint8_t> fiber_counts;

    volatile uint32_t last_active = 0;

    //用于启动时的lock
    LFLock start_lock;

    //计时器
    TimerType* m_timer = nullptr;

    //创建scheduler时，指定其可以调度的processor数量范围
    int m_max_num = 1;
    int m_min_num = 1;

    bool stopped;
    
    std::thread m_dispatch_thread;
    std::thread m_timer_thread;
    LFLock lock;

    std::mutex mutex;

    private:
    Scheduler();
    ~Scheduler();

    //将一个fiber加入到runnable中
    void add_fiber(Fiber*);
    //dispatcher线程函数
    //1.根据待执行Fiber计算负载, 将高负载的P中的协程steal一些给空载的P
    //2.侦测到阻塞的P(单个协程运行时间超过阀值), 将P中的其他协程steal给其他P
    void dispatcher_thread();

    //为每个processor分配线程
    void new_process_thread();
  };
}


//默认的scheduler
#define go_scheduler ::tiger::Scheduler::GetInstance()
  namespace tiger{
    Scheduler& Scheduler::GetInstance(){
      static Scheduler scheduler;
      return  scheduler;
    }
  }


