


#include <libgo/coroutine.h>
#include <iostream>

//Fiber切换
int main(){

  //在Fiber内部调用co_yield ---> 调用Processor::FiberYield ---> 切换running_fiber,选择下一个fiber进行运行
  //Fiber创建后，只会添加到Processor中的ready队列中 ---> 并没有开始执行 ---> 调用start()后，会processor分配线程，并开始运行
  go[]{
    std::cout << "Fiber1 start" << std::endl;
    co_yield;
    std::cout << "Fiber1 finish" << std::endl;
  };

  go[]{
    std::cout << "Fiber2 start" << std::endl;
    co_yield;
    std::cout << "Fiber2 finish" << std::endl;
    co_sched.Stop();   //回收所有的processor线程
  };
  co_sched.Start();
  return 0;
}

