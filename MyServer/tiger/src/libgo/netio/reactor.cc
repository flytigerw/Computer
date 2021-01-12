


#include "reactor.h"
#include "hookhelper.h"


namespace tiger{

  Reactor::Reactor(){

  }

  //类的静态变量声明
  std::vector<Reactor*> Reactor::g_reactors;
  //为其分配一个线程运行run
  void Reactor::init_loop_thread(){
    std::thread t([this]{
        for(;;)
          this->run();
        });
    t.detach();
  }
  //初始化生成n个reactor  最多255个
  int Reactor::InitializeReactorCount(uint8_t n){
    if(!g_reactors.empty())
      return 0;
    g_reactors.reserve(n);
    for(uint8_t i=0;i<n;i++)
      g_reactors.push_back(new EpollReactor);  //针对linux
    return 0;
  }

  Reactor& Reactor::Select(int fd){
    //默认之创建一个
    static int ignore = InitializeReactorCount(1);
    (void)(ignore);
    return *g_reactors[fd % g_reactors.size()];
  }
  bool Reactor::add(int fd,short int event_type,const Entry& entry){
    FdContext::ptr fd_ctx = FdCtxManager::GetInstance().get_fd_context(fd);
    if(!fd_ctx)
      return false;
    return fd_ctx->add(this, event_type, entry);
  }
}
