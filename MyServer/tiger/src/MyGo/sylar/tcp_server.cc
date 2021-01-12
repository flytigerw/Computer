#include "config.h"
#include "log.h"
#include "tcp_server.h"
#include "iomanager.h"

namespace sylar {

  //IO timeout 配置项
static sylar::ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout =
    sylar::Config::LookUp("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2),
            "tcp server read timeout");

static sylar::Logger::ptr g_logger = SYLAR_LOG_LOGGER("system");

TcpServer::TcpServer(sylar::IOManager* worker,
                    sylar::IOManager* io_worker,
                    sylar::IOManager* accept_worker)
    :m_worker(worker)
    ,m_ioWorker(io_worker)
    ,m_acceptWorker(accept_worker)
    ,m_recvTimeout(g_tcp_server_read_timeout->getValue())
    ,m_name("sylar/1.0.0")
    ,m_isStop(true) {
}

TcpServer::~TcpServer() {
    for(auto& i : m_socks) 
        i->close();
    m_socks.clear();
}

void TcpServer::setConf(const TcpServerConf& v) {
    m_conf.reset(new TcpServerConf(v));
}

bool TcpServer::bind(sylar::Address::ptr addr) {
    std::vector<Address::ptr> addrs;
    std::vector<Address::ptr> fails;  //记录哪些地址bind失败
    addrs.push_back(addr);
    return bind(addrs, fails);
}

bool TcpServer::bind(const std::vector<Address::ptr>& addrs,std::vector<Address::ptr>& fails ) {
    for(auto& addr : addrs) {
      //创建socket
        Socket::ptr sock = Socket::CreateTCP(addr);
        //bind socket
        if(!sock->bind(addr)) {
            SYLAR_LOG_ERROR(g_logger) << "bind fail errno="
                << errno << " errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        //listen
        if(!sock->listen()) {
            SYLAR_LOG_ERROR(g_logger) << "listen fail errno="
                << errno << " errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        m_socks.push_back(sock);
    }

    if(!fails.empty()) {
        m_socks.clear();
        return false;
    }

    for(auto& i : m_socks) {
        SYLAR_LOG_INFO(g_logger) << "type=" << m_type
            << " name=" << m_name
            << " server bind success: " << *i;
    }
    return true;
}


//接受clinet的请求,accept成功 ----> handle client
void TcpServer::startAccept(Socket::ptr sock) {
    while(!m_isStop) {
        Socket::ptr client = sock->accept();
        if(client) {
            client->setRecvTimeout(m_recvTimeout);
            m_ioWorker->schedule(std::bind(&TcpServer::handleClient,shared_from_this(), client));
        } else 
            SYLAR_LOG_ERROR(g_logger) << "accept errno=" << errno
                << " errstr=" << strerror(errno);
    }
}

bool TcpServer::start() {
    if(!m_isStop)      /////状态判断
        return true;
    m_isStop = false;
    //对每个已经bind过的listening socket调用accept
    
    for(auto& sock : m_socks) 
        m_acceptWorker->schedule(std::bind(&TcpServer::startAccept,shared_from_this(), sock));
    return true;
}

void TcpServer::stop() {
    m_isStop = true;
    auto self = shared_from_this();
    //直接传入一个回调函数即可
    m_acceptWorker->schedule([this, self]() {
        for(auto& sock : m_socks) {
            sock->cancelAll();
            sock->close();
        }
        m_socks.clear();
    });
}

//交给派生类实现
void TcpServer::handleClient(Socket::ptr client) {
    SYLAR_LOG_INFO(g_logger) << "handleClient: " << *client;
}


std::string TcpServer::toString(const std::string& prefix) {
    std::stringstream ss;
    ss << prefix << "[type=" << m_type
       << " worker=" << (m_worker ? m_worker->getName() : "")
       << " accept=" << (m_acceptWorker ? m_acceptWorker->getName() : "")
       << " recv_timeout=" << m_recvTimeout << "]" << std::endl;
    std::string pfx = prefix.empty() ? "    " : prefix;
    for(auto& i : m_socks) {
        ss << pfx << pfx << *i << std::endl;
    }
    return ss.str();
}

}
