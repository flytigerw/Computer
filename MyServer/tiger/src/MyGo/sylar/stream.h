#ifndef __SYLAR_STREAM_H__
#define __SYLAR_STREAM_H__

#include <memory>
#include "bytearray.h"

namespace sylar {


  //要屏蔽普通文件和socket文件的差异 ---> 故使用基类
class Stream {
public:
    typedef std::shared_ptr<Stream> ptr;
    virtual ~Stream() {}

    //读取数据到buffer中
    //return:>0 实际接受到的数据大小
    //       <0 出现流错误
    //       =0 流关闭
    virtual int read(void* buffer, size_t length) = 0;
    //读取数据到byteArray中
    virtual int read(ByteArray::ptr ba, size_t length) = 0;

    //读取固定长度的数据:调用read ---> 读满固定长度数据后才返回 ---> 普通read不能保证这一点
    virtual int readFixSize(void* buffer, size_t length);
    virtual int readFixSize(ByteArray::ptr ba, size_t length);

    virtual int write(const void* buffer, size_t length) = 0;

    virtual int write(ByteArray::ptr ba, size_t length) = 0;

    virtual int writeFixSize(const void* buffer, size_t length);

    virtual int writeFixSize(ByteArray::ptr ba, size_t length);

    //关闭流
    virtual void close() = 0;
};

}

#endif
