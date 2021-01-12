#include "bytearray.h"
#include <fstream>
#include <sstream>
#include <string.h>
#include <iomanip>

#include "endian.h"
#include "log.h"

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_LOGGER("system");

ByteArray::Node::Node(size_t s)
    :ptr(new char[s])
    ,next(nullptr)
    ,size(s) {
}

ByteArray::Node::Node()
    :ptr(nullptr)
    ,next(nullptr)
    ,size(0) {
}

ByteArray::Node::~Node() {
    if(ptr) 
        delete[] ptr;
}

ByteArray::ByteArray(size_t base_size)
    :m_baseSize(base_size)
    ,m_position(0)
    ,m_capacity(base_size)
    ,m_size(0)
    ,m_endian(SYLAR_BIG_ENDIAN)
    ,m_root(new Node(base_size))
    ,m_cur(m_root) {
}

ByteArray::~ByteArray() {
  //释放整个链表
    Node* tmp = m_root;
    while(tmp) {
        m_cur = tmp;
        tmp = tmp->next;
        delete m_cur;
    }
}

//字节序要和网络字节序一致
bool ByteArray::isLittleEndian() const {
    return m_endian == SYLAR_LITTLE_ENDIAN;
}

void ByteArray::setIsLittleEndian(bool val) {
    if(val) 
        m_endian = SYLAR_LITTLE_ENDIAN;
    else 
        m_endian = SYLAR_BIG_ENDIAN;
}

//单字节与网络字节序无关
void ByteArray::writeFint8  (int8_t value) {
    write(&value, sizeof(value));
}

void ByteArray::writeFuint8 (uint8_t value) {
    write(&value, sizeof(value));
}

void ByteArray::writeFint16 (int16_t value) {
  //先统一字节序
    if(m_endian != SYLAR_BYTE_ORDER) 
        value = byteswap(value);
    write(&value, sizeof(value));
}

void ByteArray::writeFuint16(uint16_t value) {
    if(m_endian != SYLAR_BYTE_ORDER) 
        value = byteswap(value);
    //整数的内存地址:char*
    write(&value, sizeof(value));
}

void ByteArray::writeFint32 (int32_t value) {
    if(m_endian != SYLAR_BYTE_ORDER) 
        value = byteswap(value);
    write(&value, sizeof(value));
}

void ByteArray::writeFuint32(uint32_t value) {
    if(m_endian != SYLAR_BYTE_ORDER) 
        value = byteswap(value);
    write(&value, sizeof(value));
}

void ByteArray::writeFint64 (int64_t value) {
    if(m_endian != SYLAR_BYTE_ORDER) 
        value = byteswap(value);
    write(&value, sizeof(value));
}

void ByteArray::writeFuint64(uint64_t value) {
    if(m_endian != SYLAR_BYTE_ORDER) 
        value = byteswap(value);
    write(&value, sizeof(value));
}


//将int类型转为无符号类型
//比如:-2,-1,0,1,2 ----> 转为 0,1,2,3,4
static uint32_t EncodeZigzag32(const int32_t& v) {
    if(v < 0) 
        return ((uint32_t)(-v)) * 2 - 1;
     else 
        return v * 2;
}

static uint64_t EncodeZigzag64(const int64_t& v) {
    if(v < 0) 
        return ((uint64_t)(-v)) * 2 - 1;
     else 
        return v * 2;
}

//4 ---> 2
//3 ---> -2
static int32_t DecodeZigzag32(const uint32_t& v) {
    return (v >> 1) ^ -(v & 1);
}

static int64_t DecodeZigzag64(const uint64_t& v) {
    return (v >> 1) ^ -(v & 1);
}


void ByteArray::writeInt32  (int32_t value) {
    writeUint32(EncodeZigzag32(value));
}

void ByteArray::writeUint32 (uint32_t value) {
  //将32位数据拆分为5组数据:4*7+4 ---> 利用每组数据的最高位
  //压缩结果:1~5字节
    uint8_t tmp[5];
    uint8_t i = 0;
    while(value >= 0x80) {
        tmp[i++] = (value & 0x7F) | 0x80; //低7位不变，最高位补1
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeInt64  (int64_t value) {
    writeUint64(EncodeZigzag64(value));
}

void ByteArray::writeUint64 (uint64_t value) {
    uint8_t tmp[10];
    uint8_t i = 0;
    while(value >= 0x80) {
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}

//先拷贝浮点型数据的内存，再以无符号型数据写入
void ByteArray::writeFloat (float value) {
    uint32_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint32(v);
}

void ByteArray::writeDouble (double value) {
    uint64_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint64(v);
}

//先写入长度，再直接写入char数组
void ByteArray::writeStringF16(const std::string& value) {
    writeFuint16(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringF32(const std::string& value) {
    writeFuint32(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringF64(const std::string& value) {
    writeFuint64(value.size());
    write(value.c_str(), value.size());
}

//********************************
void ByteArray::writeStringVint(const std::string& value) {
    writeUint64(value.size());
    write(value.c_str(), value.size());
}

//不记录长度
void ByteArray::writeStringWithoutLength(const std::string& value) {
    write(value.c_str(), value.size());
}

int8_t   ByteArray::readFint8() {
    int8_t v;
    read(&v, sizeof(v));
    return v;
}

uint8_t  ByteArray::readFuint8() {
    uint8_t v;
    read(&v, sizeof(v));
    return v;
}

#define XX(type) \
    type v; \
    read(&v, sizeof(v)); \
    if(m_endian == SYLAR_BYTE_ORDER) { \
        return v; \
    } else { \
        return byteswap(v); \
    }

int16_t  ByteArray::readFint16() {
    XX(int16_t);
}
uint16_t ByteArray::readFuint16() {
    XX(uint16_t);
}

int32_t  ByteArray::readFint32() {
    XX(int32_t);
}

uint32_t ByteArray::readFuint32() {
    XX(uint32_t);
}

int64_t  ByteArray::readFint64() {
    XX(int64_t);
}

uint64_t ByteArray::readFuint64() {
    XX(uint64_t);
}

#undef XX

int32_t  ByteArray::readInt32() {
    return DecodeZigzag32(readUint32());
}


uint32_t ByteArray::readUint32() {
    uint32_t result = 0;
    for(int i = 0; i < 32; i += 7) {
        uint8_t b = readFuint8();
        if(b < 0x80) {
            result |= ((uint32_t)b) << i;
            break;
        } else { //根据每组数据的首字节进行还原
            result |= (((uint32_t)(b & 0x7f)) << i);
        }
    }
    return result;
}

int64_t  ByteArray::readInt64() {
    return DecodeZigzag64(readUint64());
}

uint64_t ByteArray::readUint64() {
    uint64_t result = 0;
    for(int i = 0; i < 64; i += 7) {
        uint8_t b = readFuint8();
        if(b < 0x80) {
            result |= ((uint64_t)b) << i;
            break;
        } else {
            result |= (((uint64_t)(b & 0x7f)) << i);
        }
    }
    return result;
}


float ByteArray::readFloat() {
    uint32_t v = readFuint32();
    float value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

double  ByteArray::readDouble() {
    uint64_t v = readFuint64();
    double value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

std::string ByteArray::readStringF16() {
    uint16_t len = readFuint16();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readStringF32() {
    uint32_t len = readFuint32();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readStringF64() {
    uint64_t len = readFuint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readStringVint() {
    uint64_t len = readUint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

void ByteArray::clear() {
  //类似析构，但clear后可以再使用
    m_position = m_size = 0;
    m_capacity = m_baseSize;
    Node* tmp = m_root->next;
    while(tmp) {
        m_cur = tmp;
        tmp = tmp->next;
        delete m_cur;
    }
    m_cur = m_root;
    m_root->next = NULL;
}

void ByteArray::write(const void* buf, size_t size) {
    if(size == 0) 
        return;
    
    addCapacity(size);

    //当前节点已经写入的位置
    size_t npos = m_position % m_baseSize;
    //当前节点的容量
    size_t ncap = m_cur->size - npos;
    size_t bpos = 0;

    while(size > 0) {
          //逐个字节的拷贝
        if(ncap >= size) {
            memcpy(m_cur->ptr + npos, (const char*)buf + bpos, size);
            //偏移更新
            if(m_cur->size == (npos + size)) 
                m_cur = m_cur->next;
            m_position += size;
            bpos += size;
            size = 0;
        } else {
          //当前节点容量不够 ---> 会拷贝剩余部分到下一节点
            memcpy(m_cur->ptr + npos, (const char*)buf + bpos, ncap);
            m_position += ncap;
            bpos += ncap;
            size -= ncap;
            m_cur = m_cur->next;
            ncap = m_cur->size;
            npos = 0;
        }
    }

    if(m_position > m_size) 
        m_size = m_position;
}

void ByteArray::read(void* buf, size_t size) {
    if(size > getReadSize()) 
        throw std::out_of_range("not enough len");

    //类似write
    size_t npos = m_position % m_baseSize;
    size_t ncap = m_cur->size - npos;
    size_t bpos = 0;
    //可能跨节点读取拷贝
    while(size > 0) {
        if(ncap >= size) {
            memcpy((char*)buf + bpos, m_cur->ptr + npos, size);
            if(m_cur->size == (npos + size)) {
                m_cur = m_cur->next;
            }
            m_position += size;
            bpos += size;
            size = 0;
        } else {
            memcpy((char*)buf + bpos, m_cur->ptr + npos, ncap);
            m_position += ncap;
            bpos += ncap;
            size -= ncap;
            m_cur = m_cur->next;
            ncap = m_cur->size;
            npos = 0;
        }
    }
}

  //指定节点的偏移量进行读取
void ByteArray::read(void* buf, size_t size, size_t position) const {
    if(size > (m_size - position)) 
        throw std::out_of_range("not enough len");
    size_t npos = position % m_baseSize;
    size_t ncap = m_cur->size - npos;
    size_t bpos = 0;
    Node* cur = m_cur;
    while(size > 0) {
        if(ncap >= size) {
            memcpy((char*)buf + bpos, cur->ptr + npos, size);
            if(cur->size == (npos + size)) {
                cur = cur->next;
            }
            position += size;
            bpos += size;
            size = 0;
        } else {
            memcpy((char*)buf + bpos, cur->ptr + npos, ncap);
            position += ncap;
            bpos += ncap;
            size -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
    }
}

void ByteArray::setPosition(size_t v) {
    if(v > m_capacity) 
        throw std::out_of_range("set_position out of range");
    m_position = v;

    //比如:两个node -----> 8k个字节
    //m_size指向6k,剩余2k可写
    //m_position指向3k ---> m_cur指向第一个node
    //现在将m_postion指向7k ---> m_cur需要更正到指向第二个节点
    
    //m_size
    if(m_position > m_size) 
        m_size = m_position;
    
    m_cur = m_root;
    while(v > m_cur->size) {
        v -= m_cur->size;
        m_cur = m_cur->next;
    }
    if(v == m_cur->size) 
        m_cur = m_cur->next;
}


///////////////*********************///////////////
bool ByteArray::writeToFile(const std::string& name) const {
    std::ofstream ofs;
    ofs.open(name, std::ios::trunc | std::ios::binary);
    if(!ofs) {
        SYLAR_LOG_ERROR(g_logger) << "writeToFile name=" << name
            << " error , errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }

    int64_t read_size = getReadSize();
    int64_t pos = m_position;
    Node* cur = m_cur;

    //逐个节点的读取
    while(read_size > 0) {
        int diff = pos % m_baseSize;
        int64_t len = (read_size > (int64_t)m_baseSize ? m_baseSize : read_size) - diff;
        ofs.write(cur->ptr + diff, len);
        cur = cur->next;
        pos += len;
        read_size -= len;
    }
    return true;
}

bool ByteArray::readFromFile(const std::string& name) {
    std::ifstream ifs;
    ifs.open(name, std::ios::binary);
    if(!ifs) {
        SYLAR_LOG_ERROR(g_logger) << "readFromFile name=" << name
            << " error, errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }

    std::shared_ptr<char> buff(new char[m_baseSize], [](char* ptr) { delete[] ptr;});
    while(!ifs.eof()) {
        ifs.read(buff.get(), m_baseSize);
        write(buff.get(), ifs.gcount());
    }
    return true;
}

void ByteArray::addCapacity(size_t size) {
    if(size == 0) 
        return;
    size_t old_cap = getCapacity();
    if(old_cap >= size) 
        return;

    //需要扩充的容量 ---> 换算成节点数
    size = size - old_cap;
    size_t count = ceil(1.0 * size / m_baseSize);
    Node* tmp = m_root;
    while(tmp->next)   //找到末尾节点
        tmp = tmp->next;
    Node* first = NULL;
    //申请新节点
    for(size_t i = 0; i < count; ++i) {
        tmp->next = new Node(m_baseSize);
        if(first == NULL) 
            first = tmp->next;
        tmp = tmp->next;
        m_capacity += m_baseSize;
    }

    if(old_cap == 0) 
        m_cur = first;
    
}

//转为string ----> 直接读取出保存的char即可
std::string ByteArray::toString() const {
    std::string str;
    str.resize(getReadSize());
    if(str.empty()) 
        return str;
    read(&str[0], str.size(), m_position);
    return str;
}

//16进制输出
std::string ByteArray::toHexString() const {
    std::string str = toString();
    std::stringstream ss;
    for(size_t i = 0; i < str.size(); ++i) {
        if(i > 0 && i % 32 == 0) 
            ss << std::endl;
        ss << std::setw(2) << std::setfill('0') << std::hex
           << (int)(uint8_t)str[i] << " ";
    }

    return ss.str();
}


//将数据读出，存入iovec buffers中，用于scatter io
//len为数据的长度
uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len) const {
  
    len = len > getReadSize() ? getReadSize() : len;
    if(len == 0) 
        return 0;

    uint64_t size = len;

    size_t npos = m_position % m_baseSize;
    size_t ncap = m_cur->size - npos;
    struct iovec iov;
    Node* cur = m_cur;

    //byteArray本身也就是iovec buffers
    while(len > 0) {
        if(ncap >= len) {///最后一个node
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

//position给出读取数据的位置
uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers
                                ,uint64_t len, uint64_t position) const {
    len = len > getReadSize() ? getReadSize() : len;
    if(len == 0) {
        return 0;
    }

    uint64_t size = len;

    size_t npos = position % m_baseSize;
    size_t count = position / m_baseSize;
    Node* cur = m_root;
    while(count > 0) {
        cur = cur->next;
        --count;
    }

    size_t ncap = cur->size - npos;
    struct iovec iov;
    while(len > 0) {
        if(ncap >= len) {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

uint64_t ByteArray::getWriteBuffers(std::vector<iovec>& buffers, uint64_t len) {
    if(len == 0) {
        return 0;
    }
    addCapacity(len);
    uint64_t size = len;

    size_t npos = m_position % m_baseSize;
    size_t ncap = m_cur->size - npos;
    struct iovec iov;
    Node* cur = m_cur;
    while(len > 0) {
        if(ncap >= len) {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;

            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

}
