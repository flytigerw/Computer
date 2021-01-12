

#include "scheduler.h"
#include <vector>
#include <map>

namespace tiger{




  //清理list ---> 全局对象，需要保护
  std::vector<std::function<void()>>* ExitList(){
    static std::vector<std::function<void()>>* vec = new std::vector<std::function<void()>>;
    return vec;
  }
  //scheduler在退出时要做一些清理工作 
  std::mutex& ExitListMutex(){
    static std::mutex mtx;
    return mtx;
  }
  //退出时的回调
  static void onExit(){
    auto vec = ExitList();
    for(auto f:*vec)
      f();
    vec->clear();
  }
  //注册进程结束时回调
  static int InitOnExit(){
    atexit(&onExit);
    return 0;
  }

  //静态状态变量
  bool& Scheduler::IsExiting(){
    static bool exiting = false;
    return exiting;
  }

  inline std::atomic<unsigned long long>& FiberId(){
    static std::atomic<unsigned long long> id;
    return id;
  }
  Scheduler* Scheduler::Create(){
    static int s = InitOnExit();
    //先建一个Scheduler,注册其退出时的回调
    Scheduler* scheduler = new Scheduler;;
    std::unique_lock<std::mutex> lock_guard(ExitListMutex());
    auto vec = ExitList();
    //闭包
    vec->push_back([=]{delete scheduler;});
    return scheduler;
  }

  Scheduler::Scheduler(){
    //0号processor
    m_processers.push_back(new Processer(this,0));
  }

  Scheduler::~Scheduler(){
    IsExiting() = true;
    stop();
  }

  //用一个function创建一个Fiber
  void Scheduler::create_fiber(const FuncType& fcn,const FiberOption& opt){
    Fiber* f = new Fiber(fcn,opt.m_stack_size);
    //使用scheduler中delete函数来进行fiber析构 ---> 修改scheduler中的某些状态
    f->set_deleter(Deleter(&Scheduler::delete_fiber,this));
    f->m_id = ++FiberId();

    ++fiber_counts;
    add_fiber(f);
  }

  void Scheduler::delete_fiber(RefCounter* p,void* arg){
    Scheduler* self = (Scheduler*)arg;
    delete p;
    --self->fiber_counts;
  }

  void Scheduler::start(int min_num,int max_num){
    if(!start_lock.try_lock())
        throw std::logic_error("libgo repeated call Scheduler::Start");

    if(min_num < 1)
       min_num = std::thread::hardware_concurrency();  //检测硬件支持的线程数

    if(max_num == 0 || max_num < min_num)
      max_num = min_num;

    auto main_proc = m_processers[0];

    m_min_num = min_num;
    m_max_num = max_num;

    for(int i=0;i<m_min_num-1;i++)
      new_process_thread();
    //唤醒协程的定时器线程
    
    //调度协程:专门开启线程进行调度
    if(max_num > 1){
      std::thread t([this]{
          this->dispatcher_thread();
          });
      m_dispatch_thread.swap(t);
    }
  
    main_proc->schedule();
  }
 
  //调度线程的任务:
  void Scheduler::dispatcher_thread(){
    while(!stopped){
      size_t count = m_processers.size();
      size_t total_load_average = 0;
      std::multimap<size_t,size_t> actives;
      std::map<size_t,size_t> blockings;
      int active_counts = 0;

      //收集阻塞状态
      for(size_t i=0;i<count;i++){
        auto p = m_processers[i];
        if(p->is_blocking()){
          //记录阻塞processor的可运行fiber数
          blockings[i] = p->runnable_size();
          if(p->m_active)
            p->m_active = false;
        }
        if(p->m_active)
          ++active_counts;
      }

      //还可以激活几个processor 
      int active_quota = active_counts < m_min_num ? (m_min_num - active_counts) : 0;

      //统计负载值:每个processor上可运行的fiber数
      for(size_t i =0; i< count; i++){
        auto p = m_processers[i];
        size_t load_average = p->runnable_size();
        total_load_average += load_average;
        if(!p->m_active){ //针对 inactive的processor
          if(active_quota > 0 && !p->is_blocking()){
            //激活 
            p->m_active = true;
            --active_quota;
            last_active = i;
          }
        }
        //记录负载数为n的processor
        if(p->m_active){
          actives.insert({load_average,i});
          p->mark();
        }
        //processor的条件变量唤醒时刻
        if(load_average > 0 && p->is_idle())
          p->notify_condition();
      }

        //要激活0个 且可以 创建新线程
      if(actives.empty() && (int)count < m_max_num){
        new_process_thread();
        actives.insert({0,count});
        ++count;
      }

      //不能创建新线程，等待即可
      if(actives.empty())
        continue;
      //任务3:进行负载均衡，将阻塞线程的任务steal出来
     {
      SList<Fiber> idle_fibers;
      for(auto& i:blockings ){
        auto p = m_processers[i.first];
        //将阻塞的processor的fibers全部steal出来
        idle_fibers.append(p->steal(0));
      }
      if(!idle_fibers.empty()){
        //任务最少的几个processor平分
        auto range = actives.equal_range(actives.begin()->first);
        size_t avg = idle_fibers.size() / distance(range.first,range.second);
        //至少分一个
        if(avg == 0)
          avg = 1;

        std::multimap<size_t,size_t> new_actives;
        //任务较多的processor
        for(auto it = range.second;it != actives.end();++it)
          new_actives.insert(*it);
        for(auto it=range.first;it!=range.second;++it){
          //切avg个出来给任务较少的processor
          SList<Fiber> fibers = idle_fibers.cut(avg);
          if(fibers.empty())
            break;
          auto p = m_processers[it->second];
          p->add_fiber(std::move(fibers));
          //processor添加任务的情况
          new_actives.insert({p->runnable_size(),it->second});
        }
        //还有残余
        if(!idle_fibers.empty())
          m_processers[range.first->second]->add_fiber(std::move(idle_fibers));
        //????????????????????
        for(auto it=range.first; it != range.second; ++it){
          auto p = m_processers[it->second];
          new_actives.insert({p->runnable_size(),it->second});
        }
          new_actives.swap(actives);
      }
     }

    //如果还有在等待的线程，则任务多的线程中拿一些给它
      if(actives.begin()->first == 0){
          auto range = actives.equal_range(actives.begin()->first);
          std::size_t waitN = std::distance(range.first, range.second);
          if (waitN == actives.size()) 
              // 都没任务了, 不用偷了
              continue;
            
            //含有最多任务的processor
          auto max_p = m_processers[actives.rbegin()->second];
            //要steal的数量
          size_t steal_num = std::min(max_p->runnable_size()/2, waitN*1024);
          if(!steal_num)
            continue;
          auto idle_fibers = max_p->steal(steal_num);
          if(idle_fibers.empty())
            continue;
          size_t avg = idle_fibers.size() / waitN;
          if(avg == 0)
            avg = 1;

          for(auto it = range.first;it!=range.second;++it){
            SList<Fiber> fibers = idle_fibers.cut(avg);
              if(fibers.empty())
                break;
              auto p = m_processers[it->second];
              p->add_fiber(std::move(fibers));
          }
          if(!idle_fibers.empty())
            m_processers[range.first->second]->add_fiber(std::move(idle_fibers));
        }
    }
  }

  void Scheduler::new_process_thread(){
    auto p = new Processer(this,m_processers.size());
    //分配线程
    std::thread t([p]{
        p->schedule();
        });
    t.detach();
    m_processers.push_back(p);
  }

  void Scheduler::add_fiber(Fiber* f){
    auto proc = f->m_processer;
    //若fiber自所属的processor
    if(proc && proc->m_active){  //激活态才能add
      proc->add_fiber(f);
      return;
    }

    //将其添加给正在运行的processor
    proc = Processer::GetCurrentProcesser();
    //必须要处于激活态
    if(proc && proc->m_active && proc->GetCurrentScheduler() == this){
      proc->add_fiber(f);
      return;
    }

    size_t count = m_processers.size();
    size_t idx = last_active;
    //找到第一个激活态的proc,将fiber交给他
    for(size_t i = 0;i<count;++i,++idx){
      idx = idx % count;  //循环
      proc = m_processers[idx];
      if(proc && proc->m_active)
        break;
    }
    proc->add_fiber(f);
  }

  //为定时器分配线程
  void Scheduler::use_alone_timer_thread(){

    TimerType* timer = new TimerType;
    //不要锁
    if(!start_lock.try_lock()){
      timer->set_pool_size(1000,1000);
      std::thread t([timer]{
          timer->thread_run();
          });
      m_timer_thread.swap(t);
    }else{
      start_lock.unlock();
    }
    std::atomic_thread_fence(std::memory_order_acq_rel);
    m_timer = timer;
  }
  //全局性的静态timer
  static Scheduler::TimerType& GetTimer(){
    static Scheduler::TimerType *timer = new Scheduler::TimerType;
    std::thread* t = new std::thread([=]{
        timer->thread_run();
        });
    //注册退出事件
    std::unique_lock<std::mutex> lock_guard(ExitListMutex());
    auto vec = ExitList();
    vec->push_back([=]{  //程序退出时，stop timer
        timer->stop();
        if(t->joinable())
        t->join();
        });
    return *timer;
  }

  Scheduler::TimerType& Scheduler::static_timer(){
    static TimerType & timer = GetTimer();
    return timer;
  }


  Scheduler* Scheduler::Create(){
    Scheduler* sched = new Scheduler;
    //other:注册exit时要做的事
    return sched;
  }
  
  Scheduler::Scheduler(){
    //静态配置
    //一定有个Processor
    m_processers.push_back(new Processer(this,0));
  }
  Scheduler::~Scheduler(){
    stop();
  }


}
