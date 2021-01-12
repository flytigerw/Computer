


#include <libgo/coroutine.h>

#include <iostream>

void func(){
   std::cout << "Func" << std::endl;
}


struct A{

  void f1(){
    std::cout << "Member function f1" << std::endl;
  }

  void f2(){
    std::cout << "Member function f2" << std::endl;
  }
};


int main(){
    
  //Fiber创建:go + 无参的functor
  
  go func;   //函数
  go []{
    std::cout << "Lambda" << std::endl;
  };

  //成员函数
  go std::bind(&A::f1,A());
  
  //用funtion存储成员函数
  std::function<void()> f(std::bind(&A::f2,A()));

  go f;


  //指定栈大小: go co_stack(size) func
  go co_stack(10*1024*1024)[]{
    std::cout << "Stack Size" << std::endl;
  };
  
  
  //默认的scheduler为co_sched()
  
  //单独开辟一个线程运行scheduler 
  std::thread t([]{
      co_sched.Start();
      });
  t.detach();
  //sleep main fiber
  co_sleep(100);
  


}
