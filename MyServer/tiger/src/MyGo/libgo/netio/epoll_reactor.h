#pragma once

#include "reactor_event.h"
#include "reactor.h"

namespace tiger {

class EpollReactor : public Reactor
{
private:
    int m_epoll_fd;

public:
    EpollReactor();
    void run() override ;
    bool add_event(int fd, short int type, short int promise_type) override ;
    bool del_event(int fd, short int type, short int promise_type) override;

};

}
