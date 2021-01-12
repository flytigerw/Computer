#include "http_session.h"
#include "http_parser.h"

namespace sylar {
namespace http {

HttpSession::HttpSession(Socket::ptr sock, bool owner)
    :SocketStream(sock, owner) {
}

HttpRequest::ptr HttpSession::recvRequest() {
    HttpRequestParser::ptr parser(new HttpRequestParser);
    uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
    //uint64_t buff_size = 100;
    std::shared_ptr<char> buffer(           //在堆上开辟空间存储 ---> 协程栈比较小
            new char[buff_size], [](char* ptr){
                delete[] ptr;
            });

    char* data = buffer.get();
    int offset = 0;
    //header解析
    do {
      //每次读满一个node
        int len = read(data + offset, buff_size - offset);
        if(len <= 0) {
            close();
            return nullptr;
        }
        len += offset;
        //对这一块node进行解析
        size_t nparse = parser->execute(data, len);
        if(parser->hasError()) {
            close();
            return nullptr;
        }
        //调整偏移量以及结束检查
        offset = len - nparse;
        if(offset == (int)buff_size) {
            close();
            return nullptr;
        }
        if(parser->isFinished()) {
            break;
        }
    } while(true);
    int64_t length = parser->getContentLength();
    //body解析
    if(length > 0) {
        std::string body;
        body.resize(length);

        int len = 0;
        //要考虑能否装满一个node
        if(length >= offset) {
            memcpy(&body[0], data, offset);
            len = offset;
        }else {
            memcpy(&body[0], data, length);
            len = length;
        }
        length -= offset;
        if(length > 0) {
            if(readFixSize(&body[len], length) <= 0) {
                close();
                return nullptr;
            }
        }
        parser->getData()->setBody(body);
    }
    parser->getData()->init();
    return parser->getData();
}

int HttpSession::sendResponse(HttpResponse::ptr rsp) {
    std::stringstream ss;
    ss << *rsp;
    std::string data = ss.str();
    return writeFixSize(data.c_str(), data.size()); //直接将HttpResonse写入到socket流中
}

}
}
