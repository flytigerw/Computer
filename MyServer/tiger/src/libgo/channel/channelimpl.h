

#pragma once 

#include "../common/refsystem.h"
#include "../common/clock.h"

#include <memory>

//channel的实现基类接口 --> 多种实现方式


namespace tiger{

  //每种channel都有自己的id
  template<class T>
    struct ChannelImpl : public IDCounter<ChannelImpl<T>>{

    //写入数据 + 阻塞时间上限
      virtual bool push(T t,bool wait,FastSteadyClock::time_point deadline = FastSteadyClock::time_point{}) = 0;
    //读取数据 + 阻塞时间上限
      virtual bool pop(T & t, bool bWait,
            FastSteadyClock::time_point deadline = FastSteadyClock::time_point{}) = 0;
      //管道关闭
      virtual void close() = 0;
      //管道中的数据大小
      virtual std::size_t size() = 0;
      virtual bool empty() = 0;
    };


}
