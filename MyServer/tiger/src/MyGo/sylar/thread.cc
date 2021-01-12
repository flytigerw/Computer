
#include "thread.h"
#include "log.h"

namespace sylar{


  Semaphore::Semaphore(uint32_t count){
    if(sem_init(&m_semaphore,0,count))
      throw std::logic_error("sem_init error");

  }
  Semaphore::~Semaphore(){
      sem_destroy(&m_semaphore);
  }
  void Semaphore::notify(){
    //成功返回0
      if(sem_post(&m_semaphore))
        throw std::logic_error("sem sem_post error");

  }
  //等待信号量，如果信号量的值大于0,将信号量的值减1,立即返回。如果信号量的值为0,则线程阻塞。相当于P操作。成功返回0,失败返回-1。
  void Semaphore::wait(){
      if(sem_wait(&m_semaphore))
        throw std::logic_error("sem sem_wait error");
  }




  //线程局部变量，指向当前线程
  static thread_local Thread* t_thread = nullptr;
  static thread_local std::string t_thread_name = "UNKNOWN";
  
  //系统日志
  static sylar::Logger::ptr g_logger = SYLAR_LOG_LOGGER("system");

  Thread::Thread(std::function<void()> cb,const std::string& name):m_cb(cb),m_name(name){
      if(m_name.empty())
        m_name = t_thread_name;
      //this为线程运行时的参数
      int s = pthread_create(&m_thread,nullptr,&Thread::run,this);
      if(s){
        SYLAR_LOG_ERROR(g_logger) << "pthread_create fail,s = " << s
          
                                  << " name =" << m_name;
        throw std::logic_error("pthread_create error");
      }

      //create线程后，若没有启动，则会在此wait
      m_semaphore.wait();
  }

  //析构时直接将线程detach
  Thread:: ~Thread(){
    if(m_thread){
      pthread_detach(m_thread);
    }
  }


  void Thread::join(){
    if(m_thread){
      int s = pthread_join(m_thread,nullptr);
      if(s){
        SYLAR_LOG_ERROR(g_logger) << "pthread_join  fail,s = " << s
                                  << " name =" << m_name;
        throw std::logic_error("pthread_join error");
      }
      m_thread = 0;
    }
  }

  Thread* Thread::GetThis(){
    return t_thread;
  }
  const std::string& Thread::GetName(){
    return t_thread_name;          //faster,就像缓存一样
  }
  
  void* Thread::run(void* arg){
    Thread* thread = (Thread*)arg;
    t_thread  = thread;
    t_thread_name = thread->m_name;
    //内核层面的id
    thread->m_id = sylar::GetThreadId();
    //pthread_self() ---> POSIX线程库中的id
    pthread_setname_np(pthread_self(),thread->m_name.substr(0,15).c_str());

     std::function<void()> cb;
     cb.swap(thread->m_cb);  // ?????????
     //唤醒构造函数中的wait ---> 以此保证Thread创建成功后，线程一定已经运行了
     thread->m_semaphore.notify();
     cb();

     return 0; 
  }
  void Thread::setName(const std::string& name){
    if(t_thread)
      t_thread->m_name = name;
    t_thread_name = name;
  }



}

