
#ifndef __SYLAR_HTTP_PARSER_H__
#define __SYLAR_HTTP_PARSER_H__

#include "http.h"
#include "http11_parser.h"
#include "httpclient_parser.h"

namespace sylar {
namespace http {


  //core:封装底层的http_server
  //用底层的http_server解析数据:HttpRequest
class HttpRequestParser {
public:
    typedef std::shared_ptr<HttpRequestParser> ptr;
    HttpRequestParser();

    //传入文本内存地址以及文本长度进行解析
    size_t execute(char* data, size_t len);

    //解析是否完成
    int isFinished();

    //解析是否有错误
    int hasError(); 

    HttpRequest::ptr getData() const { return m_data;}

    void setError(int v) { m_error = v;}

    //获取消息体的长度
    uint64_t getContentLength();
    const http_parser& getParser() const { return m_parser;}
public:
     // 返回HttpRequest协议解析的缓存大小
    static uint64_t GetHttpRequestBufferSize();
     // 返回HttpRequest协议的最大消息体大小
    static uint64_t GetHttpRequestMaxBodySize();
private:
    /// http_parser
    http_parser m_parser;
    /// HttpRequest结构
    HttpRequest::ptr m_data;
    /// 错误码
    /// 1000: invalid method
    /// 1001: invalid version
    /// 1002: invalid field
    int m_error;
};

class HttpResponseParser {
public:
    typedef std::shared_ptr<HttpResponseParser> ptr;

    HttpResponseParser();

    size_t execute(char* data, size_t len, bool chunck);

    int isFinished();

    int hasError(); 

    HttpResponse::ptr getData() const { return m_data;}

    void setError(int v) { m_error = v;}

    uint64_t getContentLength();

    const httpclient_parser& getParser() const { return m_parser;}
public:
    static uint64_t GetHttpResponseBufferSize();

    static uint64_t GetHttpResponseMaxBodySize();
private:
    /// httpclient_parser
    httpclient_parser m_parser;
    /// HttpResponse
    HttpResponse::ptr m_data;
    /// 错误码
    /// 1001: invalid version
    /// 1002: invalid field
    int m_error;
};

}
}

#endif
