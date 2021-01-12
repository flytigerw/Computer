

#pragma once 


#include "cas_channel_impl.h"
#include "locked_channel_impl.h"


namespace tiger{


  //封装后的对外接口
  template <class T>
    class Channel{
      private: 
        mutable std::shared_ptr<ChannelImpl<T>> m_channel;

      public:
      //@capacity: capacity of channel.
      //@choose1: use CASChannelImpl if capacity less than choose1
      //@choose2: if capacity less than choose2, use ringbuffer. else use std::list.
        explicit Channel(size_t capacity =0,size_t choose1=0,size_t choose2 = 100001){
          if(capacity < choose1)
            m_channel.reset(new CASChannelImpl<T>(capacity));
          else 
            m_channel.reset(new LockedChannelImpl<T>(capacity,capacity<choose2));
        }


        //使用输入输出流简化使用
        const Channel& operator<<(T t) const{
          m_channel->push(t,true);
          return *this;
        }

        const Channel& operator>>(T& t) const{
          m_channel->pop(t,true);
          return *this;
        }

        bool try_push(T t) const{
          return m_channel->push(t,false); //false --> 不阻塞等待
        }

        bool try_pop(T& t)const{
          return m_channel->pop(t,false);
        }

        //阻塞限定时间
        template<class Rep,class Period>
          bool time_push(T t,std::chrono::duration<Rep,Period>dur)const{
            return m_channel->push(t,true,dur+FastSteadyClock::now());
          }

         bool time_push(T t, FastSteadyClock::time_point deadline) const{
            return m_channel->push(t, true, deadline);
          }

        template<class Rep,class Period>
          bool time_pop(T& t,std::chrono::duration<Rep,Period>dur)const{
            return m_channel->pop(t,true,dur+FastSteadyClock::now());
          }

         bool time_pop(T& t, FastSteadyClock::time_point deadline) const{
            return m_channel->pop(t, true, deadline);
          }


         //channel是否被共享
         bool unique() const{
           return m_channel.unique();
         }


         void close() const {
           m_channel->close();
         }

         bool empty() const{
           return  m_channel->empty();
         }

         size_t size() const{
           return m_channel->size();
         }
    };

  template<class T>
    using co_chan = Channel<T>;


}
