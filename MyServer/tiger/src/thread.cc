#include "thread.h"
#include "log.h"


namespace tiger{

  static Logger::ptr g_logger = TIGER_LOG_LOGGER("system");
    
  //指示当前Thread对象 ----> 用于协程，充当缓存
  static thread_local Thread* cur_thread = nullptr;
  static thread_local std::string cur_thread_name = "UNKNOWN";

  Thread::Thread(const std::string& name,std::function<void()>task):m_name(name),m_task(task){
    if(m_name.empty())
      m_name = cur_thread_name;
    //创建线程资源
    //将this作为参数传递给func ----> 在function中解读为Thread,调用其task
    int s = pthread_create(&p_id,nullptr,&Thread::run,this);
    if(s){
        TIGER_LOG_ERROR(g_logger) << "pthread_create failed,s=" << s;
        throw std::logic_error("pthread_create error");
    }
    //阻塞等待 另一动作的唤醒
    m_semaphore.wait();
  }

  Thread::~Thread(){
    if(p_id)
      pthread_detach(p_id);
  }
  
  void Thread::join(){
    if(p_id){
      //先简要处理，不管线程的运行结果
      int s = pthread_join(p_id,nullptr);
      if(s){
          TIGER_LOG_ERROR(g_logger) << "pthread_join failed,s=" << s;
          throw std::logic_error("pthread_join error");
       }
       p_id = 0;  //失效
    }
  }
  void* Thread::run(void* arg){

     //将void*解读为Thread
      Thread* thread = (Thread*)arg;

      //准备工作
      //缓存更新
      cur_thread = thread;
      cur_thread_name = thread->m_name;
      //在运行时获得os_id
      thread->os_id = GetThreadId();
      pthread_setname_np(pthread_self(),thread->m_name.substr(0,15).c_str());
      std::function<void()> task;
      task.swap(thread->m_task); //采用swap原因:run执行结束后,task被析构.而run执行结束，m_task不一定析构.若m_task中含有智能指针 ----> 资源不能及时释放
      thread->m_semaphore.notify();
      task();
      return 0;
  }

  Thread* Thread::GetCurThread(){
    return cur_thread;
  }
    //用于logger中
  const std::string& Thread::GetCurThreadName(){
    return cur_thread_name;
  }

}














