#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>
#include <fcntl.h>
#include "m_reactor_event.h"


namespace tiger{
  

  enum class FdType :uint8_t{
    SOCKET,
    PIPE
  };

  template<typename R,typename F,typename... Args>
    static R CallWithoutINTR(F f,Args &&...args){
retry:
      R res = f(std::forward<Args>(args)...);
      if(res == -1 && errno == EINTR) //被信号中断进行重启
        goto retry;
      return res;
    }


  //socket fd的相关属性
  struct SocketAttribute{

    int domain = -1,type = -1,protocol = -1;

    SocketAttribute() {}
    SocketAttribute(int d, int t, int p) 
        : domain(d), type(t), protocol(p){}

    bool initialized() const { return domain != -1; }
  };



  //Event包含了fd的事件属性
  //FdContext则包含了fd更全面的属性 ---> 重点关注socket相关属性
  class FdContext : public ReactorEvent{

   private:
     int m_fd;
     FdType m_fd_type;
     SocketAttribute socket_attr;
     //是否设置NON_BLOCK flag
     bool non_blocking;  
     //超时事件
     int tcp_conn_timeout;
     long recv_timeout,send_timeout;

     public:
       explicit FdContext(int fd, FdType fd_type, bool non_blocking , SocketAttribute sockAttr);

       typedef std::shared_ptr<FdContext> ptr;

     public:
       bool is_socket()const {return m_fd_type == FdType::SOCKET;}
       bool is_tcp_socket() const{
         if(!is_socket())
           return false;
         return socket_attr.type == SOCK_STREAM &&
                (socket_attr.domain == AF_INET ||
                 socket_attr.domain == AF_INET6);
       }

       bool is_nonblocking() const{
         return non_blocking;
       }
       void set_non_blocking(bool non_block){
         non_blocking = non_block;
       }


       void set_tcp_conn_timeout(int time){
         tcp_conn_timeout = time;
       }

       int get_tcp_conn_timeout()const{
         return tcp_conn_timeout;
       }

       SocketAttribute get_socket_attr()const{
         return socket_attr;
       }

       long get_socket_timeout(int type){
         switch(type){
           case SO_RCVTIMEO:
             return recv_timeout;
           case SO_SNDTIMEO:
             return send_timeout;
         }
         return 0;
       }

       void set_socket_timeout(int type,int time){
         switch(type){
           case SO_RCVTIMEO:
             recv_timeout = time;
             break;
           case SO_SNDTIMEO:
             send_timeout = time;
         }
       }

       //将属性拷贝一份
       ptr clone(int fd){
         ptr ctx(new FdContext(fd,m_fd_type,non_blocking,socket_attr));
         ctx->tcp_conn_timeout = tcp_conn_timeout;
         ctx->recv_timeout = recv_timeout;
         ctx->send_timeout = send_timeout;
         return ctx;
       }


       //调用fcntl设置fd的O_NONBLOCK属性
       bool set_non_blocking_really(bool non_block){
         int flags = CallWithoutINTR<int>(fcntl,m_fd,F_GETFL,0);
        bool old = flags & O_NONBLOCK;
        if(old == non_block)  //原来的标志和想要的标志一致
          return old;
        CallWithoutINTR<int>(fcntl,m_fd,F_SETFL,
            non_block ? (flags | O_NONBLOCK):
                           (flags & ~ O_NONBLOCK));
        set_non_blocking(non_block);
        return true;
       }
       void close(){
         ReactorEvent::close();
       }
  };

}
