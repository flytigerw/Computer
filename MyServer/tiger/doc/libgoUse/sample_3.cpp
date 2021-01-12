


#include <libgo/coroutine.h>
#include <iostream>



//多个Fiber之间通过channel传递数据

int main(){
   
  //无缓冲区的channle : F1写入，若没有等待读的Fiber，则F1挂起，直到有Fiber读取数据
  //                    F1读取，若没有等待写的Fiber，则F1官气，直到有Fiber写入数据
  co_chan<int> ch_0;

 //channel中采用shared_ptr管理channel_impl 
 //上层channel的copy并不会导致channel_impl的copy 
  go [=]{
    ch_0 << 1;    //cpoy 到lambda中 
                  //没有读者，会阻塞
  };

  go [=]{
    int i;
    ch_0 >> i;
    std::cout << i << std::endl;
  };


  //带buffer的channel : buffer没有写满之前，不会阻塞写者
  
  co_chan<std::shared_ptr<int>> ch_01(1);

  go [=]{
    std::shared_ptr<int> p1(new int(1));
    ch_01 << p1;           //不会阻塞
    ch_01 << p1;           //写满，会阻塞

    std::cout <<  "F1 finished "<< std::endl;
  };

  go [=]{
    std::shared_ptr<int> p2;
    ch_01 >> p2;         //有数据，不会阻塞，数据从full变为full-1 ---> 会唤醒写者
    std::cout <<  "F2 read "<< std::endl;
    ch_01 >> p2;    //写者已经唤醒,可以读取数据
  };

  //超时读取数据(Fiber最多被挂起给定时间),非阻塞读取数据
  
  co_chan<int> ch_2;

  go[=]{
    int val = 0;
    //非阻塞式读取
    bool ok = ch_2.TryPop(val);

    //若在100ms内都没有读者来读取数据，则Fiber从挂起中恢复,但是数据依旧已经写入
    ok = ch_2.TimedPush(1,std::chrono::microseconds(100));

  };
   /*********************** 4. 多读多写\线程安全 ************************/
      // Channel可以同时由多个线程读写.
      // Channel是线程安全的, 因此不必担心在多线程调度协程时会出现问题.
      //
   /*********************** 5. 跨越多个调度器 ************************/
      // Channel可以自由地使用, 不必关心操作它的协程是属于哪个调度器的.
      //
   /*********************** 6. 兼容原生线程 ************************/
      // Channel不仅可以用于协程中, 还可以用于原生线程.


     //200ms后安全退出 
    std::thread([]{ co_sleep(200); co_sched.Stop();  }).detach();
    co_sched.Start();
    return 0;
}
