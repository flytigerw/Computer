#ifndef __SYLAR_SOCKET_H__
#define __SYLAR_SOCKET_H__

#include <memory>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "address.h"
#include "noncopyable.h"

namespace sylar {

  //以socket fd为中心来创建对象
  //类型:TCP,UDP,  IPv4,IPv6,UNIX  ---> 两组类型
  //状态:打开，连接，关闭
  //操作:状态涉及的操作,IO操作
class Socket : public std::enable_shared_from_this<Socket>, Noncopyable {
public:
    typedef std::shared_ptr<Socket> ptr;
    typedef std::weak_ptr<Socket> weak_ptr;

    enum Type {
        /// TCP类型
        TCP = SOCK_STREAM,
        /// UDP类型
        UDP = SOCK_DGRAM
    };

    enum Family {
        /// IPv4 socket
        IPv4 = AF_INET,
        /// IPv6 socket
        IPv6 = AF_INET6,
        /// Unix socket
        UNIX = AF_UNIX,
    };

    //使用address创建一个tcp socket对象
    static Socket::ptr CreateTCP(sylar::Address::ptr address);

    //使用address创建一个udp socket对象
    static Socket::ptr CreateUDP(sylar::Address::ptr address);

    //采取默认的0.0.0.0地址
    static Socket::ptr CreateTCPSocket();
    static Socket::ptr CreateUDPSocket();

    //IPv6版本
    static Socket::ptr CreateTCPSocket6();
    static Socket::ptr CreateUDPSocket6();

    //Unix 版本
    static Socket::ptr CreateUnixTCPSocket();
    static Socket::ptr CreateUnixUDPSocket();

    /**
     * @brief Socket构造函数
     * @param[in] family 协议簇
     * @param[in] type 类型
     * @param[in] protocol 协议
     */
    Socket(int family, int type, int protocol = 0);

    virtual ~Socket();

    //获取发送数据时阻塞的time out
    int64_t getSendTimeout();
    //设置
    void setSendTimeout(int64_t v);

    //获取接受数据时阻塞的time out
    int64_t getRecvTimeout();
    //设置
    void setRecvTimeout(int64_t v);

    //调用getsockopt查看socket fd的相关属性设置
    bool getOption(int level, int option, void* result, socklen_t* len);
    //调用setsockopt进行设置
    bool setOption(int level, int option, const void* result, socklen_t len);

    //对外接口 ---> 内部C函数调用使用void* ------ 外部C++接口则使用模板
    template<class T>
    bool getOption(int level, int option, T& result) {
        socklen_t length = sizeof(T);
        return getOption(level, option, &result, &length);
    }
    template<class T>
    bool setOption(int level, int option, const T& value) {
        return setOption(level, option, &value, sizeof(T));
    }

    /**
     * @brief 接收connect链接
     * @return 成功返回新连接的socket,失败返回nullptr
     * @pre Socket必须 bind , listen  成功
     */
    virtual Socket::ptr accept();

    //地址绑定
    virtual bool bind(const Address::ptr addr);

    /**
     * @brief 连接地址
     * @param[in] addr 目标地址
     * @param[in] timeout_ms 超时时间(毫秒)
     */
    virtual bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);

    virtual bool reconnect(uint64_t timeout_ms = -1);

    /**
     * @brief 监听socket
     * @param[in] backlog 未完成连接队列的最大长度
     * @result 返回监听是否成功
     * @pre 必须先 bind 成功
     */
    virtual bool listen(int backlog = SOMAXCONN);

    //socket 关闭
    virtual bool close();

    /**
     * @brief 发送数据
     * @param[in] buffer 待发送数据的内存
     * @param[in] length 待发送数据的长度
     * @param[in] flags 标志字
     * @return
     *      @retval >0 发送成功对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int send(const void* buffer, size_t length, int flags = 0);

    /**
     * @brief 发送数据
     * @param[in] buffers 待发送数据的内存(iovec数组)
     * @param[in] length 待发送数据的长度(iovec长度)
     * @param[in] flags 标志字
     * @return
     *      @retval >0 发送成功对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int send(const iovec* buffers, size_t length, int flags = 0);

    /**
     * @brief 发送数据
     * @param[in] buffer 待发送数据的内存
     * @param[in] length 待发送数据的长度
     * @param[in] to 发送的目标地址
     * @param[in] flags 标志字
     * @return
     *      @retval >0 发送成功对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int sendTo(const void* buffer, size_t length, const Address::ptr to, int flags = 0);

    /**
     * @brief 发送数据
     * @param[in] buffers 待发送数据的内存(iovec数组)
     * @param[in] length 待发送数据的长度(iovec长度)
     * @param[in] to 发送的目标地址
     * @param[in] flags 标志字
     * @return
     *      @retval >0 发送成功对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int sendTo(const iovec* buffers, size_t length, const Address::ptr to, int flags = 0);

    /**
     * @brief 接受数据
     * @param[out] buffer 接收数据的内存
     * @param[in] length 接收数据的内存大小
     * @param[in] flags 标志字
     * @return
     *      @retval >0 接收到对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int recv(void* buffer, size_t length, int flags = 0);

    /**
     * @brief 接受数据
     * @param[out] buffers 接收数据的内存(iovec数组)
     * @param[in] length 接收数据的内存大小(iovec数组长度)
     * @param[in] flags 标志字
     * @return
     *      @retval >0 接收到对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int recv(iovec* buffers, size_t length, int flags = 0);

    /**
     * @brief 接受数据
     * @param[out] buffer 接收数据的内存
     * @param[in] length 接收数据的内存大小
     * @param[out] from 发送端地址
     * @param[in] flags 标志字
     * @return
     *      @retval >0 接收到对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int recvFrom(void* buffer, size_t length, Address::ptr from, int flags = 0);

    /**
     * @brief 接受数据
     * @param[out] buffers 接收数据的内存(iovec数组)
     * @param[in] length 接收数据的内存大小(iovec数组长度)
     * @param[out] from 发送端地址
     * @param[in] flags 标志字
     * @return
     *      @retval >0 接收到对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int recvFrom(iovec* buffers, size_t length, Address::ptr from, int flags = 0);

    /**
     * @brief 获取远端地址
     */
    Address::ptr getRemoteAddress();

    /**
     * @brief 获取本地地址
     */
    Address::ptr getLocalAddress();

    int getFamily() const { return m_family;}
    int getType() const { return m_type;}
    int getProtocol() const { return m_protocol;}
    bool isConnected() const { return m_isConnected;}

    //socket fd是否有效 ----> 默认socket fd = -1 ---> 无效
    bool isValid() const;

    /**
     * @brief 返回Socket错误
     */
    int getError();

    /**
     * @brief 输出信息到流中
     */
    virtual std::ostream& dump(std::ostream& os) const;

    virtual std::string toString() const;
    //获得scoket fd
    int getSocket() const { return m_sock;}

    //事件的cancel:读写事件正在阻塞等待数据.cancel意味着将他们强制唤醒 ---> 依旧没有数据可读 ----> 返回
    bool cancelRead();
    bool cancelWrite();
    bool cancelAccept();
    bool cancelAll();
protected:
    /**
     * @brief 初始化socket
     */
    void initSock();

    /**
     * @brief 创建socket
     */
    void newSock();
    
    //用一个socket fd显示初始化 ----> 可用于accept()中
    virtual bool init(int sock);
protected:
    /// socket句柄
    int m_sock;
    /// 协议簇
    int m_family;
    /// 类型
    int m_type;
    /// 协议
    int m_protocol;
    /// 是否连接
    bool m_isConnected;
    /// 本地地址
    Address::ptr m_localAddress;
    /// 远端地址
    Address::ptr m_remoteAddress;
};

std::ostream& operator<<(std::ostream& os, const Socket& sock);

}

#endif
