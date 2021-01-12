#pragma once

#include "m_reactor_event.h"
#include "epoll_reactor.h"

namespace tiger{


  class Reactor{
    private:
      //全局的Reactor组
      static std::vector<Reactor*> g_reactors;

    public:
      static Reactor& Select(int fd);
      //初始创建n个reactor
      static int InitializeReactorCount(uint8_t n);

    public:
      typedef ReactorEvent::Entry Entry;
      Reactor();
      //为fd添加一个想要监听的事件类型 --> 先获取fd对应的fd_ctx ---> fd_ctx含有event属性，调用event的add进行添加
      bool add(int fd,short int event_type,const Entry& entry);

    public:
      virtual void run() = 0;
      virtual bool add_event(int fd,short int add_type,short int promise_type) = 0;
      virtual bool del_event(int fd,short int del_type,short int promise_type) = 0;


    protected:
      void init_loop_thread();
  };

}
