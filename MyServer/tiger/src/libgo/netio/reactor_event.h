#pragma once 

#include <vector>
#include <sys/epoll.h>
#include <memory>
#include "../scheduler/processor.h" 

namespace tiger{
class EpollReactor; 

class Event{

  public:
    
    struct Entry{
        Processer::SuspendEntry fiber_info;
        //一个Fiber可能有多个IO请求
        std::shared_ptr<short int> m_types; 
        int idx;       //每个Entry都有索引下标?
        Entry(){}
        Entry(Processer::SuspendEntry suspended_fiebr,const std::shared_ptr<short int>& types,int id):fiber_info(suspended_fiebr),m_types(types),idx(id){}
        bool operator==(const Entry& rhs){
          return fiber_info == rhs.fiber_info && 
                 m_types == rhs.m_types &&
                 idx == rhs.idx;
        }
    };

  public:
    typedef std::vector<Entry> EntryList;
  private:
    std::mutex m_mtx;
    int m_fd;
    short int m_type;
    EntryList read,write,error,read_write;

  public:
    Event(int fd);
    //在阻塞队列上增加一个fiber
    bool add(EpollReactor* reactor,short int type,const Entry& entry);
    //唤醒某条阻塞队列上的fiber
    void trigger(EpollReactor* reactor,short int type);

    //选择某条阻塞队列
    EntryList& select_list(short int type);

    void trigger_without_lock(short int type,EntryList& list);

    //插入的回滚
    void roll_back(EntryList & entryList, Entry const& entry);
    
    void check_expire(EntryList & entryList);

    void close();

};

}

