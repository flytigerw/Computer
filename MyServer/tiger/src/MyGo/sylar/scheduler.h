#ifndef SCHEDULER
#define SCHEDULER 

#include "fiber.h"
#include "thread.h"
#include <vector>
#include <list>
#include <atomic>
#include "log.h"

//用线程池容纳一组线程
//每一线程可调度m个协程
namespace sylar{
  
  class Scheduler{
    public:
      typedef Mutex MutexType;
      typedef std::shared_ptr<Scheduler> ptr;
      //use_caller:为true表明，将Scheduler的调用者线程也纳入到调度管理中
      Scheduler(size_t threads = 1, bool use_caller =true,const std::string& name="");
      virtual ~Scheduler();

      const std::string& getName() const{return m_name;}

      static Scheduler* GetThis();  //需要知道 current fiber的调度器是哪一个
      static Fiber* GetMainFiber();     //运行调度器的主协程

      void start();
      void stop();

      //在协程调度中调度执行协程
      template<class FiberOrCb>
        void schedule(FiberOrCb fc,int thread=-1){
       SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "schedule";
          bool notifyThread = false;
          {
            MutexType::LockGuard lock_guard(m_mutex);
            notifyThread = scheduleNoLcok(fc,thread);
          }
          if(notifyThread)
            notify();
        }
      //放一组任务
      template<class InputIterator>
        void schedule(InputIterator first,InputIterator last){
          bool notifyThread = false;
          {
            MutexType::LockGuard lock_guard(m_mutex);
            while(first != last){
              notifyThread = scheduleNoLcok(&(*first),-1) || notifyThread;
              ++first;
            }
          }

          if(notifyThread)
            notify();
        }

    void switchTo(int thread = -1);
    std::ostream& dump(std::ostream& os);

    protected:
     virtual void notify();
     void run();
     virtual bool stopping();
     void setThis();
     //协程无任务可调度时执行idle协程
     virtual void idle();
     bool hasIdleThreads() { return m_idle_threads > 0;}
    private:
      template<class FiberOrCb>
        //将协程任务压入到m_fibers中
        bool scheduleNoLcok(FiberOrCb fc,int thread){
          bool notifyThread = m_fibers.empty();   //为空表明目前没有任务需要执行 ----> 现在来了任务，就需要通知
          FiberAndThread ft(fc,thread);
          if(ft.fiber || ft.m_cb)
            m_fibers.push_back(ft);
          return notifyThread;
        }

    private:
      struct FiberAndThread{
          Fiber::ptr fiber;
          std::function<void()> m_cb;
          int thread_id;
          FiberAndThread(Fiber::ptr f,int id):fiber(f),thread_id(id){}   //希望fiber运行在哪一个thread上
          FiberAndThread(Fiber::ptr* f,int id):thread_id(id){
              fiber.swap(*f); //涉及到引用计数
          }   
          FiberAndThread(std::function<void()> cb,int id):m_cb(cb),thread_id(id){}   //fiber可以体现为一个function
          FiberAndThread(std::function<void()>* cb,int id):thread_id(id){
              m_cb.swap(*cb);
          }   
          FiberAndThread():thread_id(-1){}
          void reset(){
            fiber = nullptr;
            m_cb = nullptr;
            thread_id = -1;
          }
      };
    protected:
      std::vector<Thread::ptr> m_threads;
      MutexType m_mutex;
      std::list<FiberAndThread> m_fibers;
      std::string m_name;
      Fiber::ptr m_rootFiber ;

    protected:

      //所有的线程Id
      std::vector<int> m_threadIds;
      //总的线程数
      size_t m_total_threads {0};
      //活跃的线程数
      std::atomic<size_t> m_active_threads {0};
      //空闲的线程数
      std::atomic<size_t> m_idle_threads {0} ;
      bool m_stopping = true;  
      //是否能自动停止
      bool m_autoStop = false;
      int m_rootThreadId = 0;  
  };
}

#endif
