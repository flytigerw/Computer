
#include <poll.h>
#include <algorithm>
#include "m_reactor_event.h"



namespace tiger{


  //ReactorEvent是以poll为基础的
  //添加事件
bool ReactorEvent::add(Reactor * reactor, short int type,  const Entry& entry)
{
    std::unique_lock<std::mutex> lock_guard(m_mtx);
    EntryList & entry_list = select_list(type);
    check_expire(entry_list);
    entry_list.push_back(entry);     //添加到相关队列中

    short int add_type = type & (POLLIN | POLLOUT);

    if (add_type == 0)
        add_type |= POLLERR;

   
    //比如m_type为0
    short int promise_type = m_type | add_type;   
    //add_type 为 0 时，promise_type == m_type 
    if (promise_type != m_type) {
        if (!reactor->add_event(m_fd, add_type, promise_type)) {
            // add error.
            roll_back(entry_list, entry);
            return false;
        }
        
        m_type = promise_type;   //m_type更新 --> 反映当前监听的事件类型
        return true;
    }
    return true;
}


//回滚 --> 撤销插入
void ReactorEvent::roll_back(EntryList & entry_list, Entry const& entry)
{
    auto it = std::find(entry_list.begin(), entry_list.end(), entry);
    if (entry_list.end() != it)
        entry_list.erase(it);
}


void ReactorEvent::close(){
 //唤醒所有的队列
  trigger(nullptr,POLLNVAL);
}

//type:已到达的事件类型 ---> 唤醒对应队列上的fiber
void ReactorEvent::trigger(Reactor * reactor, short int type)
{
    std::unique_lock<std::mutex> lock_guard(m_mtx);

    short int error_type = POLLERR | POLLHUP | POLLNVAL;
    short int promise_type = 0;

    short int check = POLLIN | error_type;

    //若type含有error_type，则也唤醒阻塞的fiber
    if (type & check) {                              //read 就绪 
        if (!read.empty())
          trigger_without_lock(type & check, read);  //唤醒
    } else if (!read.empty()) {                      //read并没有就绪
        promise_type |= POLLIN;                 
    }

    check = POLLOUT | error_type;
    if (type & check) {
        if (!write.empty())
          trigger_without_lock(type & check, write);
    } else if (!write.empty()) {
        promise_type |= POLLOUT;
    }

    check = POLLIN | POLLOUT | error_type;
    if (type & check) {
        if (!read_write.empty())
        trigger_without_lock(type & check, read_write);
    } else if (!read_write.empty()) {
        promise_type |= POLLIN|POLLOUT;
    }

    check = error_type;
    if (type & check) {
        if (!error.empty())
          trigger_without_lock(type & check, error);
    } else if (!error.empty()) {
        promise_type |= POLLERR;
    }

    //promise_type代表没有发生的事件类型
    //m_type:想要监听的事件类型
    //m_type - promise_type : 已到来的事件类型
    //若promise_type为0,则进行删除.否则进行修改
    short int del_type = m_type & ~promise_type;
    if (promise_type != m_type) {
        if (reactor && reactor->delete_event(fd_, del_type, promise_type))
            m_type = promise_type;
        return ;
    }
  }
void ReactorEvent::trigger_without_lock(short int event_type, EntryList & entry_list)
{
    for (Entry & entry : entry_list) {
        entry.arr_event_types.get()[entry.idx] = event_type;  //一个fiber可能阻塞多个IO事件
        Processer::WakeUp(entry.fiber_info);           //唤醒某一条队列上的fiber
    }
    entry_list.clear();

}

ReactorEvent::EntryList & ReactorEvent::select_list(short int type)
{
    EntryList * res_list = nullptr;
    if ((type & POLLIN) && (type & POLLOUT)) {
        res_list = &read_write;
    } else if (type & POLLIN) {
        res_list = &read;
    } else if (type & POLLOUT) {
        res_list = &write;
    } else {
        res_list = &error;
    }
    return *res_list;
}

void ReactorEvent::check_expire(EntryList & entry_list)
{
    entry_list.erase(std::remove_if(entry_list.begin(), entry_list.end(),  //删除无效的suspended_fiber
                    [](Entry & entry){
                    return entry.fiber_info.IsExpire();
                    }),
                    entry_list.end());
}

}
