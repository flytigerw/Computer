#include "address.h"
#include "log.h"
#include <netdb.h>
#include <cstddef>
#include <ifaddrs.h>


namespace sylar{

static sylar::Logger::ptr g_logger = SYLAR_LOG_LOGGER("system");


template<class T>
  //创建bits位的掩码:低bits位全为1
  static T CreateMask(uint32_t bits){
    return (1 << (sizeof(T)*8 - bits)) - 1;
  }

//计算一个数的二进制中，1的个数
template<class T>
  static uint32_t CountBytes(T value){
    uint32_t res = 0;
    for(;value;res++)
      value &= value-1;   //采用位运算，逐个清除value的1
    return res;
  }

//getAddr返回sockaddr,里面含有sa_family字段
int Address::getFamily() const {
    return getAddr()->sa_family;
}

//送入strignstream中即可
std::string Address::toString() const {
    std::stringstream ss;
    insert(ss);
    return ss.str();
}


bool Address::operator<(const Address& rhs) const{
    //逐位比较
    socklen_t minlen = std::min(getAddrLen(),rhs.getAddrLen());
    int result = memcmp(getAddr(),rhs.getAddr(),minlen);
    if(result < 0)
      return  true;
    else if(result > 0)
      return false;
    //短者优先
    else if(getAddrLen() < rhs.getAddrLen())
      return true;
    return false;
}

bool Address::operator==(const Address& rhs) const {
    return getAddrLen() == rhs.getAddrLen()
        && memcmp(getAddr(), rhs.getAddr(), getAddrLen()) == 0;
}

bool Address::operator!=(const Address& rhs) const {
    return !(*this == rhs);
}

Address::ptr Address::Create(const sockaddr* addr, socklen_t addrlen) {
    if(addr == nullptr) {
        return nullptr;
    }
    Address::ptr result;
    //根据sa_family来判断当前地址属于哪种类型
    //返回基类指针，调用者需要通过dynamic_cast转为派生类指针
    switch(addr->sa_family) {
        case AF_INET:
            result.reset(new IPv4Address(*(const sockaddr_in*)addr));
            break;
        case AF_INET6:
            result.reset(new IPv6Address(*(const sockaddr_in6*)addr));
            break;
        default:
            result.reset(new UnknownAddress(*addr));
            break;
    }
    return result;
}


//关键:使用addrinfo完成address与host的转换
bool Address::Lookup(std::vector<Address::ptr>& result, const std::string& host,
                     int family, int type, int protocol) {
    addrinfo hints, *results, *next;
    hints.ai_flags = 0;
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;
    hints.ai_addrlen = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    std::string node;
    const char* service = NULL;

    //检查 ipv6address serivce
    if(!host.empty() && host[0] == '[') {
        const char* endipv6 = (const char*)memchr(host.c_str() + 1, ']', host.size() - 1);
        if(endipv6) {
            //TODO check out of range
            if(*(endipv6 + 1) == ':') 
                service = endipv6 + 2;
            node = host.substr(1, endipv6 - host.c_str() - 1);
        }
    }
    //参考www.sylar.top:80
    ///切割得到域名和service
    //node保存域名
    if(node.empty()) {
      //找到第一个:的位置 ----> :后面的内容就是service
        service = (const char*)memchr(host.c_str(), ':', host.size());
        if(service) {
            if(!memchr(service + 1, ':', host.c_str() + host.size() - service - 1)) {
                node = host.substr(0, service - host.c_str());
                ++service;
            }
        }
    }

    if(node.empty()) 
        node = host;
    
    int error = getaddrinfo(node.c_str(), service, &hints, &results);
    if(error) {
        SYLAR_LOG_DEBUG(g_logger) << "Address::Lookup getaddress(" << host << ", "
            << family << ", " << type << ") err=" << error << " errstr="
            << gai_strerror(error);
        return false;
    }
    //可以导致返回多个addrinfo结构的情形有以下2个：
    //如果与hostname参数关联的地址有多个，那么适用于所请求地址簇的每个地址都返回一个对应的结构。
    //如果service参数指定的服务支持多个套接口类型，那么每个套接口类型都可能返回一个对应的结构，具体取决于hints结构的ai_socktype成员
    next = results;
    //将getaddrinfo() 得到的地址保存
    while(next) {
        result.push_back(Create(next->ai_addr, (socklen_t)next->ai_addrlen));
        next = next->ai_next;
    }
    freeaddrinfo(results);
    return !result.empty();
}

Address::ptr Address::LookupAny(const std::string& host,
                                int family, int type, int protocol) {
    std::vector<Address::ptr> result;
    if(Lookup(result, host, family, type, protocol)) {
        return result[0];
    }
    return nullptr;
}

IPAddress::ptr Address::LookupAnyIPAddress(const std::string& host,
                                int family, int type, int protocol) {
    std::vector<Address::ptr> result;
    if(Lookup(result, host, family, type, protocol)) {
        for(auto& i : result) {
            IPAddress::ptr v = std::dynamic_pointer_cast<IPAddress>(i); //-----判断基类pointer是否指向某个派生类对象
            if(v) {
                return v;
            }
        }
    }
    return nullptr;
}
//了解即可，其使用ifaddr中的相关方法
bool Address::GetInterfaceAddresses(std::multimap<std::string,std::pair<Address::ptr, uint32_t> >& result,int family) {
    struct ifaddrs *next, *results;
    if(getifaddrs(&results) != 0) {
        SYLAR_LOG_DEBUG(g_logger) << "Address::GetInterfaceAddresses getifaddrs "
            " err=" << errno << " errstr=" << strerror(errno);
        return false;
    }

    try {
        for(next = results; next; next = next->ifa_next) {
            Address::ptr addr;
            uint32_t prefix_len = ~0u;
            if(family != AF_UNSPEC && family != next->ifa_addr->sa_family) {
                continue;
            }
            switch(next->ifa_addr->sa_family) {
                case AF_INET:
                    {
                        addr = Create(next->ifa_addr, sizeof(sockaddr_in));
                        uint32_t netmask = ((sockaddr_in*)next->ifa_netmask)->sin_addr.s_addr;
                        prefix_len = CountBytes(netmask);
                    }
                    break;
                case AF_INET6:
                    {
                        addr = Create(next->ifa_addr, sizeof(sockaddr_in6));
                        in6_addr& netmask = ((sockaddr_in6*)next->ifa_netmask)->sin6_addr;
                        prefix_len = 0;
                        for(int i = 0; i < 16; ++i) {
                            prefix_len += CountBytes(netmask.s6_addr[i]);
                        }
                    }
                    break;
                default:
                    break;
            }

            if(addr) {
                result.insert(std::make_pair(next->ifa_name,
                            std::make_pair(addr, prefix_len)));
            }
        }
    } catch (...) {
        SYLAR_LOG_ERROR(g_logger) << "Address::GetInterfaceAddresses exception";
        freeifaddrs(results);
        return false;
    }
    freeifaddrs(results);
    return !result.empty();
}

bool Address::GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >&result,const std::string& iface, int family) {
    if(iface.empty() || iface == "*") {
        if(family == AF_INET || family == AF_UNSPEC) {
            result.push_back(std::make_pair(Address::ptr(new IPv4Address()), 0u));
        }
        if(family == AF_INET6 || family == AF_UNSPEC) {
            result.push_back(std::make_pair(Address::ptr(new IPv6Address()), 0u));
        }
        return true;
    }

    std::multimap<std::string,std::pair<Address::ptr, uint32_t> > results;
    if(!GetInterfaceAddresses(results, family)) 
        return false;

    auto its = results.equal_range(iface);
    for(; its.first != its.second; ++its.first) 
        result.push_back(its.first->second);
    return !result.empty();
}

//将任意类型的string address转为想要的格式版本
IPAddress::ptr IPAddress::Create(const char* address, uint16_t port) {
    addrinfo hints, *results;
    memset(&hints, 0, sizeof(addrinfo));

    //imortant -------------------------------------
    //AI_NUMERICHOST 传递给getaddrinfo函数的nodename参数必须是数字字符串
    //AF_UNSPEC 地址系列未指定
    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;
    
    //getaddrinfo函数能够处理名字到地址以及服务到端口这两种转换，返回的是一个sockaddr结构的链表
    int error = getaddrinfo(address, NULL, &hints, &results);
    if(error) {
        SYLAR_LOG_DEBUG(g_logger) << "IPAddress::Create(" << address
            << ", " << port << ") error=" << error
            << " errno=" << errno << " errstr=" << strerror(errno);
        return nullptr;
    }

    try {
      //因为Unknown address并不继承IPAddress,故并不在IP address做地址类型判断
      //而是向上追溯到共同的祖先，在其中进行类型判断。利用多态，使得只用一个基类指针就可指向所有派生类对象
        IPAddress::ptr result = std::dynamic_pointer_cast<IPAddress>(
                Address::Create(results->ai_addr, (socklen_t)results->ai_addrlen)); //ai_addr为第一个sockaddr --> IPv4 or IPv6 or Unix
        if(result) {
            result->setPort(port);
        }
        freeaddrinfo(results);
        return result;
    } catch (...) {
        freeaddrinfo(results);
        return nullptr;
    }
}




IPv4Address::IPv4Address(uint32_t address, uint16_t port) {
  //设置自己的m_addr即可
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    //转为网络序
    m_addr.sin_port = byteswapOnLittleEndian(port);
    m_addr.sin_addr.s_addr = byteswapOnLittleEndian(address);
}
IPv4Address::IPv4Address(const sockaddr_in& address) {
    m_addr = address;
}

sockaddr* IPv4Address::getAddr() {
    return (sockaddr*)&m_addr;
}

const sockaddr* IPv4Address::getAddr() const {
    return (sockaddr*)&m_addr;
}


socklen_t IPv4Address::getAddrLen() const {
    return sizeof(m_addr);
}

//格式化:decimal化地输入到stream中
std::ostream& IPv4Address::insert(std::ostream& os) const {

    uint32_t addr = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);
    //点分十进制
    os << ((addr >> 24) & 0xff) << "."
       << ((addr >> 16) & 0xff) << "."
       << ((addr >> 8) & 0xff) << "."
       << (addr & 0xff);
    os << ":" << byteswapOnLittleEndian(m_addr.sin_port);
    return os;
}


IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len) {
    if(prefix_len > 32) 
        return nullptr;
    sockaddr_in baddr(m_addr);
    //mask的低位为Host id,全为1, |= --> broadcastAddress
    //my_addr.sin_addr.s_addr   =   inet_addr("192.168.0.1");获取IPv4地址的常用方法
    baddr.sin_addr.s_addr |= byteswapOnLittleEndian(
            CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(baddr));
    
}

IPAddress::ptr IPv4Address::networkAddress(uint32_t prefix_len) {
    if(prefix_len > 32) {
        return nullptr;
    }
    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr &= ~byteswapOnLittleEndian(
            CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(baddr));
}

IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len) {
    sockaddr_in subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin_family = AF_INET;
    //通过~运算将低位置0,
    subnet.sin_addr.s_addr = ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(subnet));
}

uint32_t IPv4Address::getPort() const {
    return byteswapOnLittleEndian(m_addr.sin_port);
}

void IPv4Address::setPort(uint16_t v) {
    m_addr.sin_port = byteswapOnLittleEndian(v);
}

IPv4Address::ptr IPv4Address::Create(const char* address, uint16_t port) {
    IPv4Address::ptr rt(new IPv4Address);
    rt->m_addr.sin_port = byteswapOnLittleEndian(port);
    //核心:调用inet_pton进行转换 person to network
    //type address dest
    int result = inet_pton(AF_INET, address, &rt->m_addr.sin_addr);
    if(result <= 0) {
        SYLAR_LOG_DEBUG(g_logger) << "IPv4Address::Create(" << address << ", "
                << port << ") rt=" << result << " errno=" << errno
                << " errstr=" << strerror(errno);
        return nullptr;
    }
    return rt;
}

IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
    m_addr.sin6_port = byteswapOnLittleEndian(port);
    memcpy(&m_addr.sin6_addr.s6_addr, address, 16);
}
IPv6Address::IPv6Address() {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const sockaddr_in6& address) {
    m_addr = address;
}
sockaddr* IPv6Address::getAddr() {
    return (sockaddr*)&m_addr;
}

const sockaddr* IPv6Address::getAddr() const {
    return (sockaddr*)&m_addr;
}
socklen_t IPv6Address::getAddrLen() const {
    return sizeof(m_addr);
}
uint32_t IPv6Address::getPort() const {
    return byteswapOnLittleEndian(m_addr.sin6_port);
}

void IPv6Address::setPort(uint16_t v) {
    m_addr.sin6_port = byteswapOnLittleEndian(v);
}

std::ostream& IPv6Address::insert(std::ostream& os) const {
    os << "[";
    uint16_t* addr = (uint16_t*)m_addr.sin6_addr.s6_addr;
    bool used_zeros = false;
    for(size_t i = 0; i < 8; ++i) {
        if(addr[i] == 0 && !used_zeros) {
            continue;
        }
        if(i && addr[i - 1] == 0 && !used_zeros) {
            os << ":";
            used_zeros = true;
        }
        if(i) {
            os << ":";
        }
        os << std::hex << (int)byteswapOnLittleEndian(addr[i]) << std::dec;
    }

    if(!used_zeros && addr[7] == 0) {
        os << "::";
    }

    os << "]:" << byteswapOnLittleEndian(m_addr.sin6_port);
    return os;
}


IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len) {
    sockaddr_in6 baddr(m_addr);
    baddr.sin6_addr.s6_addr[prefix_len / 8] |=
        CreateMask<uint8_t>(prefix_len % 8);
    for(int i = prefix_len / 8 + 1; i < 16; ++i) {
        baddr.sin6_addr.s6_addr[i] = 0xff;
    }
    return IPv6Address::ptr(new IPv6Address(baddr));
}
IPAddress::ptr IPv6Address::networkAddress(uint32_t prefix_len) {
    sockaddr_in6 baddr(m_addr);
    baddr.sin6_addr.s6_addr[prefix_len / 8] &=
        CreateMask<uint8_t>(prefix_len % 8);
    for(int i = prefix_len / 8 + 1; i < 16; ++i) {
        baddr.sin6_addr.s6_addr[i] = 0x00;
    }
    return IPv6Address::ptr(new IPv6Address(baddr));
}


IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len) {
    sockaddr_in6 subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin6_family = AF_INET6;
    subnet.sin6_addr.s6_addr[prefix_len /8] =
        ~CreateMask<uint8_t>(prefix_len % 8);

    for(uint32_t i = 0; i < prefix_len / 8; ++i) {
        subnet.sin6_addr.s6_addr[i] = 0xff;
    }
    return IPv6Address::ptr(new IPv6Address(subnet));
}

IPv6Address::ptr IPv6Address::Create(const char* address, uint16_t port) {
    IPv6Address::ptr rt(new IPv6Address);
    rt->m_addr.sin6_port = byteswapOnLittleEndian(port);
    //核心:调用inet_pton
    int result = inet_pton(AF_INET6, address, &rt->m_addr.sin6_addr);
    if(result <= 0) {
        SYLAR_LOG_DEBUG(g_logger) << "IPv6Address::Create(" << address << ", "
                << port << ") rt=" << result << " errno=" << errno
                << " errstr=" << strerror(errno);
        return nullptr;
    }
    return rt;
}

//取出struct中某成员的长度
static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un*)0)->sun_path) - 1;

UnixAddress::UnixAddress() {
  //offsetof:计算结构体中memeber成员的偏移位置
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    //整个结构体的最大长度
    m_length = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
}

//path的memecpy
UnixAddress::UnixAddress(const std::string& path) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_length = path.size() + 1;

    //path[0] = '\0' ---> abstract name 
    if(!path.empty() && path[0] == '\0') 
        --m_length;
    
    if(m_length > sizeof(m_addr.sun_path)) 
        throw std::logic_error("path too long");
    
    memcpy(m_addr.sun_path, path.c_str(), m_length);
    m_length += offsetof(sockaddr_un, sun_path);
}

void UnixAddress::setAddrLen(uint32_t v) {
    m_length = v;
}

sockaddr* UnixAddress::getAddr() {
    return (sockaddr*)&m_addr;
}

const sockaddr* UnixAddress::getAddr() const {
    return (sockaddr*)&m_addr;
}

socklen_t UnixAddress::getAddrLen() const {
    return m_length;
}

std::string UnixAddress::getPath() const {
    std::stringstream ss;
    
    if(m_length > offsetof(sockaddr_un, sun_path) && m_addr.sun_path[0] == '\0')  //针对abstract name
        ss << "\\0" << std::string(m_addr.sun_path + 1,m_length - offsetof(sockaddr_un, sun_path) - 1);
    else 
        ss << m_addr.sun_path;
    return ss.str();
}

std::ostream& UnixAddress::insert(std::ostream& os) const {
    if(m_length > offsetof(sockaddr_un, sun_path) && m_addr.sun_path[0] == '\0') 
        return os << "\\0" << std::string(m_addr.sun_path + 1,
                m_length - offsetof(sockaddr_un, sun_path) - 1);
    return os << m_addr.sun_path;
}

UnknownAddress::UnknownAddress(int family) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sa_family = family;
}

UnknownAddress::UnknownAddress(const sockaddr& addr) {
    m_addr = addr;
}

sockaddr* UnknownAddress::getAddr() {
    return (sockaddr*)&m_addr;
}

const sockaddr* UnknownAddress::getAddr() const {
    return &m_addr;
}

socklen_t UnknownAddress::getAddrLen() const {
    return sizeof(m_addr);
}

std::ostream& UnknownAddress::insert(std::ostream& os) const {
    os << "[UnknownAddress family=" << m_addr.sa_family << "]";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Address& addr) {
    return addr.insert(os);
}

}
