#include "../src/thread.h"
#include <iostream>
#include <vector>


tiger::SpinMutex m_mutex;
long x = 0;
void task(){
  for(long i=0;i<100000000;i++){
    tiger::SpinMutex::LockGuard lock_guard(m_mutex);
    ++x;
    ++x;
  }
}


int main(){
  std::vector<tiger::Thread::ptr> m_threads;
  for(int i=0;i<5;i++)
      m_threads.push_back(tiger::Thread::ptr(new tiger::Thread("thread"+std::to_string(i),task)));
  for(int i = 0; i<5; i++){
    m_threads[i]->join();
  }
  std::cout << x << std::endl;
}
