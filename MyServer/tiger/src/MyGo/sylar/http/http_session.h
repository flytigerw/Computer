
#ifndef __SYLAR_HTTP_SESSION_H__
#define __SYLAR_HTTP_SESSION_H__

#include "../../sylar/streams/socket_stream.h"
#include "http.h"

namespace sylar {
namespace http {

  //服务端的IO流对象:read request,write response
class HttpSession : public SocketStream {
public:
    /// 智能指针类型定义
    typedef std::shared_ptr<HttpSession> ptr;

    HttpSession(Socket::ptr sock, bool owner = true);

    //接受请求:生成HttpRequest对象
    HttpRequest::ptr recvRequest();
    //发出相应
    int sendResponse(HttpResponse::ptr rsp);
};

}
}

#endif
