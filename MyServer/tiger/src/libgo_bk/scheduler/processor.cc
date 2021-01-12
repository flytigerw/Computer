#include "processor.h"
#include "scheduler.h"



namespace tiger{

  
  uint32_t cycle_timeout_us = 100 * 1000; 
//静态局部变量 thread_local性质 

 //当前线程中的Processor 
 Processer* & Processer::GetCurrentProcesser(){
   static thread_local Processer* cur_proc = nullptr;
   return cur_proc;
 }
 //该Processor所属的Scheduler
 Scheduler* Processer::GetCurrentScheduler(){
   auto p = GetCurrentProcesser();
   return p ? p->m_scheduler : nullptr;
 }
 
 Fiber* Processer::GetCurrentFiber(){
   auto p = GetCurrentProcesser();
   return p ? p->running_fiber : nullptr;
 }

 //回到主fiber:由Scheduler静态调用
 void Processer::fiber_yield(){
   Fiber* cur_fiber = GetCurrentFiber();
   ++cur_fiber->yield_count;
   cur_fiber->swap_out();
 }

 //static性质
 void Processer::FiberYield(){
    auto proc = GetCurrentProcesser();
    if (proc) 
      proc->fiber_yield();
 }

 void Processer::add_fiber(Fiber* f){
      //添加到就绪队列中
      //先上锁
   std::unique_lock<LockType> lock_guard(ready.lock_ref());
   ready.push_without_lock(f);
      //通知
      if(idle)
        cond.notify_all();
      else 
        notified = true;
 }

 void Processer::add_fiber(M_List&& mlist){
      //移动到自己的队列中
   std::unique_lock<LockType> lock_guard(ready.lock_ref());
      ready.push_without_lock(std::move(mlist));
      if(idle)
        cond.notify_all();
      else 
        notified = true;
 }

 bool Processer::add_fibers_to_run_queue(){
    runnable.push(ready.pop_all());
    //assert
    return true;
 }
 void Processer::schedule(){
   //更新cur_processer
    GetCurrentProcesser() = this;

    //1)先得到一个可运行的fiber
    //2)调度执行 ---> 执行完后的处理
    while(!m_scheduler->is_stopped()){

     //从runnable队列中，取出一个赋给running_fiber
     runnable.front(running_fiber);
      if(!running_fiber){  //没有运行的fiber ---> 将ready中fiber转移到runnable中
        if(add_fibers_to_run_queue())
          runnable.front(running_fiber);
        if(!running_fiber){
          //阻塞在条件变量上
          wait_on_conditon();
          add_fibers_to_run_queue();
          continue;
        }
      }
    
    // 每轮调度只加有限次数新协程, 防止新协程创建新协程产生死循环
      add_new_quota = 1;

    while(running_fiber && !m_scheduler->is_stopped()){
      //调度执行该fiber 
      running_fiber->m_state = FiberState::RUNNABLE;
      running_fiber->m_processer = this;

      ++schedule_count;

      //切换执行
      running_fiber->swap_in();
      
      //执行完以后，根据状态，进行下一步选择
      switch(running_fiber->m_state){
        case FiberState::RUNNABLE: //调用了yield

          
          /* 1)选择run_fiber队列中的下一个 : next
           * 2)next为空，表明running_fiber队列中没有
           * */
          {
            //running_fiber可能在
          std::unique_lock<LockType> lock_guard(ready.lock_ref());
          auto next = (Fiber*)running_fiber->next;
          if(next){
            //找到下一个要运行的
            running_fiber= next;
            break;
           }
          if(add_new_quota < 1 || ready.empty_without_lock()){  //加了一次就开始第二轮 
            running_fiber = nullptr; //回到最开始的when
          }
          else{
            lock_guard.unlock();
            //向runnable中补充弹药
            if(add_fibers_to_run_queue()){
              runnable.next(running_fiber,running_fiber);
              --add_new_quota;                        // 一轮加一次running_fiber
            }else{  //补充弹药失败
          std::unique_lock<LockType> lock_guard2(ready.lock_ref());
              running_fiber = nullptr;
            }
          }
         }
          break;
        case FiberState::BLOCK:
          /* 阻塞的时机?
           * 比如IO阻塞
           * 直接运行下一个fiber ---> 运行完以后重新调度
           * */
           {
             std::unique_lock<LockType> lock_guard(runnable.lock_ref());
             running_fiber = next_fiber;
             next_fiber = nullptr;
            }
               break;
        
        case FiberState::DONE: 
        default:
          /* 与RUNABLE是类似的处理
           * 只不过会将running_fiber从run队列中删除
           * */
            {
              //next_fiber不为null的时机
              runnable.next(running_fiber,next_fiber);

              //补充弹药
              if(!next_fiber && add_new_quota > 0){
                if(add_fibers_to_run_queue()){
                  runnable.next(running_fiber,next_fiber);
                       -- add_new_quota;
                }
              }
              //垃圾清理
              if(gc_queue.size() > 16)
                gc();
              gc_queue.push(running_fiber);
             std::unique_lock<LockType> lock_guard(runnable.lock_ref());
             running_fiber = next_fiber;
             next_fiber = nullptr;
            }
         }
      }
    }
}

 //调用时机:runnable队列为空,阻塞等待任务
 void Processer::wait_on_conditon(){
   //垃圾回收
   gc();
   std::unique_lock<LockType> lock_guard(runnable.lock_ref());
   if(notified){
     //虚假唤醒?
        notified= false;
        return ;
   }
   idle = true;   //其他processor 可以来steal fibers
   cond.wait(lock_guard);
   idle = false;  
 }
 void Processer::notify_condition(){
   std::unique_lock<LockType> lock_guard(runnable.lock_ref());
   if(idle)
     cond.notify_all();
   else 
     notified = true;
 }

 void Processer::gc(){
    //删除gc_queue中每一个待回收的fiber
    auto list = gc_queue.pop_all();
    for(auto& fiber : list)
      fiber.decrement();   //递减引用计数
    list.clear();
 }
  std::size_t Processer::runnable_size()
 {
    return runnable.size() + ready.size();
 }




Processer::M_List Processer::steal(size_t n){
   if(n > 0){
      /*  1)其他processor从当前processor中steal n和filer ---> 从ready队列中取出
       *  2)要考虑是否steal running_fiber
       * */
     auto slist = ready.pop_back(n);
     if(slist.size() >= n)
       return slist;
     //ready中个数不够，就从runnable中steal
     std::unique_lock<LockType> lock_guard(runnable.lock_ref());
     
      bool push_running_fiber = false, push_next_fiber = false;
      if(running_fiber)
        push_running_fiber = runnable.erase_without_lock(running_fiber,true) || slist.erase(running_fiber,nullptr);
      
      if(next_fiber)
        push_next_fiber = runnable.erase_without_lock(next_fiber,true) || slist.erase(next_fiber,nullptr);
      
      //从runnable中steal剩余部分
      auto slist2 = runnable.pop_back_without_lock(n-slist.size());

      if (push_running_fiber)
            runnable.push_without_lock(running_fiber);
      if (push_next_fiber)
            runnable.push_without_lock(next_fiber);
      lock_guard.unlock();

      slist2.append(std::move(slist));
      if(!slist2.empty())
        return slist2;
   }else{  //n = 0时，steal all

     auto slist = ready.pop_all();
     std::unique_lock<LockType> lock_guard(runnable.lock_ref());
     
      bool push_running_fiber = false, push_next_fiber = false;
      if(running_fiber)
        push_running_fiber = runnable.erase_without_lock(running_fiber,true) || slist.erase(running_fiber,nullptr);
      
      if(next_fiber)
        push_next_fiber = runnable.erase_without_lock(next_fiber,true) || slist.erase(next_fiber,nullptr);
      
      //从runnable中steal剩余部分
      auto slist2 = runnable.pop_all_without_lock();

      if (push_running_fiber)
            runnable.push_without_lock(running_fiber);
      if (push_next_fiber)
            runnable.push_without_lock(next_fiber);
      lock_guard.unlock();

      slist2.append(std::move(slist));
      if(!slist2.empty())
        return slist2;
   }
 }

 //挂起在运行的fiber ---> 交给其所属的processor
 Processer::SuspendEntry Processer::Suspend(){
    Fiber* cur_fiber = GetCurrentFiber();
    //assert 
    return cur_fiber->m_processer->suspend_my_fiber(cur_fiber);
 }
 Processer::SuspendEntry Processer::Suspend(time_t time){
      SuspendEntry entry = Suspend();
      GetCurrentScheduler();
 }

 Processer::SuspendEntry Processer::suspend_my_fiber(Fiber* f){
    //状态修改
    f->m_state = FiberState::BLOCK;
    uint64_t id = ++f->suspend_id;
    //将其从runnable队列中删除,并得到下一个运行的fiber
     std::unique_lock<LockType> lock_guard(runnable.lock_ref());
     runnable.next_without_lock(running_fiber,next_fiber);
     runnable.erase_without_lock(running_fiber, false);
    //并将其添加到waiting队列中
     waiting.push_without_lock(running_fiber,false);
     //返回suspend entry 
    return SuspendEntry{ WeakPtr<Fiber>(f), id };
    //  return SuspendEntry{ std::weak_ptr<Fiber>(f), id };
 }

  //被suspend的fiber是否还存在?
   bool Processer::IsSuspendExpire(const SuspendEntry& entry){
     //转为强引用指针
     IncursivePtr<Fiber> fiber = entry.m_fiber.lock();
     if(!fiber)
       return true;
     //id不一致? ---> fiber被复用了
     if(entry.m_id != fiber.get()->m_id)
       return true;
     return false;
   }

  bool Processer::WakeUp(const SuspendEntry& entry,const std::function<void()>& fctor){
     IncursivePtr<Fiber> fiber = entry.m_fiber.lock();
     if(!fiber)
       return false;

     auto proc = fiber->m_processer;

     return proc ? proc->wakeup_my_fiber(fiber,entry.m_id,fctor) : false;
  }

  bool Processer::IsCoroutine(){
    return !!GetCurrentFiber();
  }
  bool Processer::wakeup_my_fiber(const IncursivePtr<Fiber>& fiber,uint64_t id,const std::function<void()>& fctor){

    Fiber* f = fiber.get();
    std::unique_lock<LockType> lock_guard(waiting.lock_ref());
    if(f->m_id != id)
      return false;
    ++f->suspend_id;
    //唤醒时的回调
    if(fctor)
      fctor();
    //从waiting中删除,添加到runnable中 ---> false表明都不修改引用计数
    bool ret = waiting.erase_without_lock(f,false);
    size_t size_after_push = runnable.push_without_lock(f,false);
    //唤醒刚才阻塞的兄弟
    if(size_after_push == 1 && GetCurrentProcesser() != this){
      lock_guard.unlock();
      notify_condition();
    }
    return true;
  }
void Processer::mark(){
  //有fiber正在运行且mark_schedule != schedule_count 
  //schedule_count更新点:schedule()
  //mark_schedule更新点:mark 
  //何时会相等? ---> 在一次process()内调用两次mark()
  if(running_fiber && mark_schedule != schedule_count){
    mark_schedule = schedule_count;
    mark_tick = now_ms();  //打上时间戳
  }
}
  bool Processer::is_blocking(){
    //mark_swith为0 ---> 不可能为阻塞态
    if(!mark_schedule || mark_schedule != schedule_count)
      return false;
    return now_ms() > mark_tick+cycle_timeout_us; //超过了最大运行时间 ---> 阻塞
  }
int64_t Processer::now_ms(){
  return std::chrono::duration_cast<std::chrono::microseconds>(FastSteadyClock::now().time_since_epoch()).count();
}
 


}

