

#pragma once 

#include "../scheduler/processor.h"

namespace tiger{

//一个fd有多种事件类型，read,write,error,readAndwrite
//每种事件类型上可能有多个fiber在阻塞等待IO --> 形成list
//list的每个元素Entry就应该记录fiber的相关信息
 class Reactor;
 class ReactorEvent{
  public:
    
    struct Entry{
        Processer::SuspendEntry fiber_info;
        //一个Fiber可能有多个IO请求 ? 
        std::shared_ptr<short int> arr_event_types; 
        int idx;       //每个Entry都有索引下标?
        Entry(){}

        Entry(Processer::SuspendEntry suspended_fiebr,const std::shared_ptr<short int>& types,int id)
             :fiber_info(suspended_fiebr),
              arr_event_types(types),
              idx(id){}

        bool operator==(const Entry& rhs){
          return fiber_info == rhs.fiber_info && 
                 arr_event_types == rhs.arr_event_types &&
                 idx == rhs.idx;
        }
    };

  public:
    typedef std::vector<Entry> EntryList;
  private:
    std::mutex m_mtx;
    int m_fd;                 //一个Event对应一个fd
    short int m_type;
    EntryList read,write,error,read_write;

  public:
    ReactorEvent(int fd);
    //在阻塞队列上增加一个fiber
    bool add(Reactor* reactor,short int type,const Entry& entry);
    //唤醒某条阻塞队列上的fiber
    void trigger(Reactor* reactor,short int type);

    //选择某条阻塞队列
    EntryList& select_list(short int type);

    void trigger_without_lock(short int type,EntryList& list);

    //插入的回滚
    void roll_back(EntryList & entryList, Entry const& entry);
    
    void check_expire(EntryList & entryList);


    void close();

  };

}
