#ifndef __SYLAR_SOCKET_STREAM_H__
#define __SYLAR_SOCKET_STREAM_H__

#include "../../sylar/stream.h"
#include "../../sylar/socket.h"
#include "../../sylar/thread.h"
#include "../../sylar/iomanager.h"

namespace sylar {

  //socket流:对原始的socket io函数进行简单封装 ---> 将IO流封装为对象
class SocketStream : public Stream {
public:
    typedef std::shared_ptr<SocketStream> ptr;

    SocketStream(Socket::ptr sock, bool owner = true);

    ~SocketStream();

    virtual int read(void* buffer, size_t length) override;
    virtual int read(ByteArray::ptr ba, size_t length) override;
    virtual int write(const void* buffer, size_t length) override;
    virtual int write(ByteArray::ptr ba, size_t length) override;
    virtual void close() override;
    Socket::ptr getSocket() const { return m_socket;}
    bool isConnected() const ;

    Address::ptr getRemoteAddress();
    Address::ptr getLocalAddress();
    std::string getRemoteAddressString();
    std::string getLocalAddressString();
protected:
    /// Socket类
    Socket::ptr m_socket;
    /// 是否主控   ---> 若为owner,则在析构时会关闭句柄。若不为owner,则不关闭
    bool m_owner;
};

}

#endif
