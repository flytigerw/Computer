

#ifndef ADDRESS_H
#define ADDRESS_H 


#include <memory>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>
#include "util.h"


namespace sylar{

  class IPAddress;
  class Address{
    public:
      typedef std::shared_ptr<Address> ptr;
      ~Address(){}

      //哪一类型的地址:UNIX,IPV4,IPV6
      int getFamily() const;

      virtual const sockaddr* getAddr() const = 0;
      virtual sockaddr* getAddr() = 0;
      virtual socklen_t getAddrLen() const = 0;

      //送入stream中，方便输出
      virtual std::ostream& insert(std::ostream&) const = 0;
      std::string toString() const;

      bool operator<(const Address&) const;
      bool operator==(const Address&) const;
      bool operator!=(const Address&) const;

      //通过原始的sockaddr结构创建Address对象
      static Address::ptr Create(const sockaddr* addr,socklen_t addrlen);

      //解析host(比如 www.sylar.top[:http]),提取出域名与服务名，将域名和服务名转为IP地址+port，并生成Address对象 --> 可能有多个
      static bool Lookup(std::vector<Address::ptr>& result, const std::string& host,
                          int family = AF_INET, int type = 0, int protocol = 0);
      //从Lookup中的查找结果中，返回一个满足条件的即可
      static Address::ptr LookupAny(const std::string& host,
                                    int family = AF_INET, int type = 0, int protocol = 0);
      //类似，返回一个IPAddress
      static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);
      //获取指定网卡对应的地址以及掩码位数
      //一块网卡可能由多个IP地址，比如IPv4,IPv6 ....
      static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >&result
                    ,const std::string& iface, int family = AF_INET);
      //获取本机上所有网卡的地址和掩码位数
      //用map维护网卡名称到 <网卡地址，掩码位数>的映射
      static bool GetInterfaceAddresses(std::multimap<std::string,std::pair<Address::ptr, uint32_t> >& result,
                    int family = AF_INET);
  };

  class IPAddress: public Address{
    public:
      typedef std::shared_ptr<IPAddress> ptr;

      //通过IP地址获得广播地址，子网掩码，网络地址
      virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
      virtual IPAddress::ptr networkAddress(uint32_t prefix_len) = 0;
      virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

      virtual uint32_t getPort() const = 0;
      virtual void setPort(uint16_t v) = 0;
      static IPAddress::ptr Create(const char* address, uint16_t port=0);
  };

  class IPv4Address:public IPAddress{

    public:
      typedef std::shared_ptr<IPv4Address> ptr;
      //0.0.0.0
      IPv4Address(uint32_t address= INADDR_ANY,uint16_t port=0);
      IPv4Address(const sockaddr_in& address);
      sockaddr* getAddr() override;
      const sockaddr* getAddr() const override;
      socklen_t getAddrLen() const override;
      std::ostream& insert(std::ostream& os) const override;

      IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
      IPAddress::ptr networkAddress(uint32_t prefix_len) override;
      IPAddress::ptr subnetMask(uint32_t prefix_len) override;
      uint32_t getPort() const override;
      void setPort(uint16_t v) override;
      
      //将一个字符串转为IPv4实例
      static IPv4Address::ptr Create(const char*,uint16_t port=0);
    private:
    sockaddr_in m_addr;

  };
  class IPv6Address:public IPAddress{

    public:
      typedef std::shared_ptr<IPv6Address> ptr;
      //0.0.0.0
      IPv6Address(const uint8_t address[16],uint16_t port=0);
      IPv6Address();
      IPv6Address(const sockaddr_in6& address);
      const sockaddr* getAddr() const override;
      sockaddr* getAddr() override;
      socklen_t getAddrLen() const override;
      std::ostream& insert(std::ostream& os) const override;

      IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
      IPAddress::ptr networkAddress(uint32_t prefix_len) override;
      IPAddress::ptr subnetMask(uint32_t prefix_len) override;
      uint32_t getPort() const override;
      void setPort(uint16_t v) override;
      static IPv6Address::ptr Create(const char*,uint16_t port=0);
    private:
    sockaddr_in6 m_addr;

  };

 class UnixAddress : public Address {
  public:
     typedef std::shared_ptr<UnixAddress> ptr;

      UnixAddress();

      UnixAddress(const std::string& path);

     const sockaddr* getAddr() const override;
     sockaddr* getAddr() override;
     socklen_t getAddrLen() const override;
     void setAddrLen(uint32_t v);
     std::string getPath() const;
     std::ostream& insert(std::ostream& os) const override;
  private:
     sockaddr_un m_addr;
     socklen_t m_length;
};
  class UnknownAddress : public Address {
  public:
     typedef std::shared_ptr<UnknownAddress> ptr;
      UnknownAddress(int family);
      UnknownAddress(const sockaddr& addr);
      const sockaddr* getAddr() const override;
      sockaddr* getAddr() override;
      socklen_t getAddrLen() const override;
      std::ostream& insert(std::ostream& os) const override;
  private:
      sockaddr m_addr;
  };

//能够使用iostream进行输出
std::ostream& operator<<(std::ostream& os, const Address& addr);
}

#endif
