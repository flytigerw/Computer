#ifndef __SYLAR_HTTP_HTTP_SERVER_H__
#define __SYLAR_HTTP_HTTP_SERVER_H__

#include "../../sylar/tcp_server.h"
#include "http_session.h"
#include "servlet.h"

namespace sylar {
namespace http {

  //继承自TCP server，重点在于handle client的实现
class HttpServer : public TcpServer {
public:
    /// 智能指针类型
    typedef std::shared_ptr<HttpServer> ptr;

    HttpServer(bool keepalive = false
               ,sylar::IOManager* worker = sylar::IOManager::GetThis()
               ,sylar::IOManager* io_worker = sylar::IOManager::GetThis()
               ,sylar::IOManager* accept_worker = sylar::IOManager::GetThis());
    ServletDispatch::ptr getServletDispatch() const { return m_dispatch;}
    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v;}

    virtual void setName(const std::string& v) override;
protected:
    virtual void handleClient(Socket::ptr client) override;
private:
    /// 是否支持长连接  ---> 一个连接可支持多个请求与发送
    bool m_isKeepalive;
    /// Servlet分发器
    ServletDispatch::ptr m_dispatch;
};

}
}

#endif
