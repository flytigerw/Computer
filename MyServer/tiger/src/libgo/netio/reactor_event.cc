#include <poll.h>
#include "reactor_event.h"


namespace tiger{


bool Event::add(EpollReactor * reactor, short int type,  const Entry& entry)
{
    std::unique_lock<std::mutex> lock_guard(m_mtx);
    EntryList & entry_list = select_list(type);
    check_expire(entry_list);
    entry_list.push_back(entry);     //添加到相关队列中

    short int add_type = type & (EPOLLIN | EPOLLOUT);

    if (add_type == 0)
        add_type |= EPOLLERR;

    short int promise_type = m_type | add_type;    //考虑事件类型的修改
    if (promise_type != m_type) {
        if (!reactor->add_event(fd_, add_type, promise_type)) {
            // add error.
            roll_back(entry_list, entry);
            return false;
        }
        
        m_type = promise_type;   //m_type的逐步更新
        return true;
    }
    return true;
}


//回滚 --> 撤销插入
void Event::roll_back(EntryList & entry_list, Entry const& entry)
{
    auto it = std::find(entry_list.begin(), entry_list.end(), entry);
    if (entry_list.end() != it)
        entry_list.erase(it);
}


void Event::trigger(EpollReactor * reactor, short int type)
{
    std::unique_lock<std::mutex> lock_guard(m_mtx);

    short int errortype = EPOLLERR | EPOLLHUP;
    short int promise_type = 0;

    short int check = EPOLLIN | errortype;
    if (type & check) {                         //read 就绪
        if (!read.empty())
        trigger_without_lock(type & check, read);
    } else if (!read.empty()) {                 //read并没有就绪
        promise_type |= EPOLLIN;
    }

    check = EPOLLOUT | errortype;
    if (type & check) {
        if (!write.empty())
        trigger_without_lock(type & check, write);
    } else if (!write.empty()) {
        promise_type |= EPOLLOUT;
    }

    check = EPOLLIN | EPOLLOUT | errortype;
    if (type & check) {
        if (!read_write.empty())
        trigger_without_lock(type & check, read_write);
    } else if (!read_write.empty()) {
        promise_type |= EPOLLIN|EPOLLOUT;
    }

    check = errortype;
    if (type & check) {
        if (!error.empty())
        trigger_without_lock(type & check, error);
    } else if (!error.empty()) {
        promise_type |= EPOLLERR;
    }

    //若以上任何事件命中，则不会delete对应的type
    short int del_type = m_type & ~promise_type;
    if (promise_type != m_type) {
        if (reactor && reactor->delete_event(fd_, del_type, promiseEvent))
            m_type = promise_type;
        return ;
    }
  }
void Event::trigger_without_lock(short int event_type, EntryList & entry_list)
{
    for (Entry & entry : entry_list) {
        entry.m_types.get()[entry.idx] = event_type;  //一个fiber可能阻塞多个IO事件
        Processer::WakeUp(entry.fiber_info);           //唤醒某一条队列上的fiber
    }
    entry_list.clear();

}

Event::EntryList & Event::select_list(short int type)
{
    EntryList * res_list = nullptr;
    if ((type & EPOLLIN) && (type & EPOLLOUT)) {
        res_list = &read_write;
    } else if (type & EPOLLIN) {
        res_list = &read;
    } else if (type & EPOLLOUT) {
        res_list = &write;
    } else {
        res_list = &error;
    }
    return *res_list;
}

void Event::check_expire(EntryList & entry_list)
{
    entry_list.erase(std::remove_if(entry_list.begin(), entry_list.end(),  //删除无效的suspended_fiber
                    [](Entry & entry){
                    return entry.fiber_info.IsExpire();
                    }),
                    entry_list.end());
}

void close(){
 //唤醒正在等待的fiber
  trigger(nullptr,0);
}

}
