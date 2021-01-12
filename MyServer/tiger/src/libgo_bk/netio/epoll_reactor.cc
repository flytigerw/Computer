
#include <thread>
#include <poll.h>
#include "hookhelper.h"
#include "epoll_reactor.h"
#include "fdcontext.h"



namespace tiger{


const char* EpollOp2Str(int op){
  switch (op){
#define XX(str)\
  case str : return #str
  XX(EPOLL_CTL_ADD);
  XX(EPOLL_CTL_MOD);
  XX(EPOLL_CTL_DEL);
#undef XX
  default:
    return "NONE";
  }
}         


uint32_t Poll2Epoll(short int poll_type)
{
    uint32_t epoll_type = 0;
    if (poll_type & POLLIN)
        epoll_type |= EPOLLIN;
    if (poll_type & POLLOUT)
        epoll_type |= EPOLLOUT;
    if (poll_type & POLLERR)
        epoll_type |= EPOLLERR;
    if (poll_type & POLLHUP)
        epoll_type |= EPOLLHUP;
    return epoll_type;
}


short int Epoll2Poll(uint32_t epoll_type)
{
    short int poll_type = 0;
    if (epoll_type & EPOLLIN)
        poll_type |= POLLIN;
    if (epoll_type & EPOLLOUT)
        poll_type |= POLLOUT;
    if (epoll_type & EPOLLERR)
        poll_type |= POLLERR;
    if (epoll_type & EPOLLHUP)
        poll_type |= POLLHUP;
    return poll_type;
}
  
//ctor:创建一个epoll instance
//为reactor分配一个线程
EpollReactor::EpollReactor(){
  m_epoll_fd = epoll_create(1024);
  init_loop_thread();
}

bool EpollReactor::add_event(int fd, short int type, short int promise_type)
{
    struct epoll_event ev;
    
    //ET触发
    ev.events = (Poll2Epoll(promise_type)) | EPOLLET;
    ev.data.fd = fd;
    //若二者不一致 ---> 修改
    //若type == promise_type.表明m_type为0
    int op = (type == promise_type) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    
    int res = CallWithoutINTR<int>(::epoll_ctl, m_epoll_fd, op, fd, &ev);
    return res == 0;
}



//promise_type才是要删除的
bool EpollReactor::del_event(int fd, short int del_type, short int promise_type)
{
    struct epoll_event ev;
    ev.events = (Poll2Epoll(promise_type) | EPOLLET);
    ev.data.fd = fd;
    int op = promise_type == 0 ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
    int res = CallWithoutINTR<int>(::epoll_ctl, m_epoll_fd, op, fd, &ev);
    return res == 0;
}

void EpollReactor::run()
{
    const int max_num = 1024;
    struct epoll_event evlist[max_num];
    int n = CallWithoutINTR<int>(::epoll_wait,m_epoll_fd ,evlist,max_num, 10);
    //处理ready fd的等待队列 ---> 唤醒等待的fiber
    for (int i = 0; i < n; ++i) {
        struct epoll_event & ev = evlist[i];
        int fd = ev.data.fd;
        FdContext::ptr ctx = FdCtxManager::GetInstance().get_fd_context(fd);
        if (!ctx)
            continue;
        ctx->trigger(this, Epoll2Poll(ev.events));
    }
}

}
