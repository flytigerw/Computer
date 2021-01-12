#ifndef __SYLAR_BYTEARRAY_H__
#define __SYLAR_BYTEARRAY_H__

#include <memory>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>



//网络数据都为二进制，在用户层和数据层中间添加一层，用于数据的转换: 序列化和反序列化

namespace sylar {

/**
 * @brief 二进制数组,提供基础类型的序列化,反序列化功能
 */
class ByteArray {
public:
    typedef std::shared_ptr<ByteArray> ptr;

    // 内部采用链表的数据结构来表示内存:起点+大小+下一块地址      -----> 字节数组char* ---> 使用delete []
    
    // 默认情况下size为4k
    struct Node {
        Node(size_t s);
        Node();
        ~Node();

        /// 内存块地址指针
        char* ptr;
        /// 下一个内存块地址
        Node* next;
        /// 内存块大小
        size_t size;
    };

    /**
     * @brief 使用指定长度的内存块构造ByteArray
     * @param[in] base_size 内存块大小
     */
    ByteArray(size_t base_size = 4096);
    ~ByteArray();

    //写入固定长度
    void writeFint8  (int8_t value);
    void writeFuint8 (uint8_t value);
    void writeFint16 (int16_t value);
    void writeFuint16(uint16_t value);
    void writeFint32 (int32_t value);
    void writeFuint32(uint32_t value);
    void writeFint64 (int64_t value);
    void writeFuint64(uint64_t value);

    //根据数据所占的实际长度进行写入
    //比如32位数据可能只占2byte
    void writeInt32  (int32_t value);
    void writeUint32 (uint32_t value);

    void writeInt64  (int64_t value);

    void writeUint64 (uint64_t value);

    //写入浮点型:无符号固定长度数据
    void writeFloat  (float value);

    void writeDouble (double value);


    //string类型:char数组 ---> byte

    //写入string 类型:用前两个字节16位作为长度
    void writeStringF16(const std::string& value);
    void writeStringF32(const std::string& value);
    void writeStringF64(const std::string& value);

    /**
     * @brief 写入std::string类型的数据,用无符号Varint64作为长度类型
     * @post m_position += Varint64长度 + value.size()
     *       如果m_position > m_size 则 m_size = m_position
     */
    void writeStringVint(const std::string& value);

    /**
     * @brief 写入std::string类型的数据,无长度
     * @post m_position += value.size()
     *       如果m_position > m_size 则 m_size = m_position
     */
    void writeStringWithoutLength(const std::string& value);

    //读取
    //要判断整个链表中 可读取的数据  与 想要读取的数据 之间的大小比较
    //若数据不够，则跑出out_of_range异常
    
    //读取固定内存长度
    int8_t   readFint8();
    uint8_t  readFuint8();
    int16_t  readFint16();
    uint16_t readFuint16();
    int32_t  readFint32();
    uint32_t readFuint32();
    int64_t  readFint64();
    uint64_t readFuint64();

    //根据数据类型实际所占用的内存长度进行读取
    int32_t  readInt32();
    uint32_t readUint32();
    int64_t  readInt64();
    uint64_t readUint64();

    //浮点型
    float    readFloat();
    double   readDouble();
    std::string readStringF16();
    std::string readStringF32();
    std::string readStringF64();
    std::string readStringVint();


    
    //---------------------------供上层调用----------------------------------------
    void clear();
    /////下层的IO函数:
    void write(const void* buf, size_t size);
    void read(void* buf, size_t size);

    //可自选位置进行读取
    void read(void* buf, size_t size, size_t position) const;

    //读指针的当前位置
    size_t getPosition() const { return m_position;}
    void setPosition(size_t v);

    //文件操作
    bool writeToFile(const std::string& name) const;
    bool readFromFile(const std::string& name);

    //内存块大小
    size_t getBaseSize() const { return m_baseSize;}

    //byteArray中可读取的数据
    size_t getReadSize() const { return m_size - m_position;}

    //是否为小端
    bool isLittleEndian() const;
    void setIsLittleEndian(bool val);

    //将[m_position,m_size)中的数据转为string
    std::string toString() const;

   
    // 将ByteArray里面的数据[m_position, m_size)转成16进制的std::string(格式:FF FF FF)
    std::string toHexString() const;

    /**
     * @brief 获取可读取的缓存,保存成iovec数组
     * @param[out] buffers 保存可读取数据的iovec数组
     * @param[in] len 读取数据的长度,如果len > getReadSize() 则 len = getReadSize()
     * @return 返回实际数据的长度
     */
    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len = ~0ull) const;

    /**
     * @brief 获取可读取的缓存,保存成iovec数组,从position位置开始
     * @param[out] buffers 保存可读取数据的iovec数组
     * @param[in] len 读取数据的长度,如果len > getReadSize() 则 len = getReadSize()
     * @param[in] position 读取数据的位置
     * @return 返回实际数据的长度
     */
    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const;

    /**
     * @brief 获取可写入的缓存,保存成iovec数组
     * @param[out] buffers 保存可写入的内存的iovec数组
     * @param[in] len 写入的长度
     * @return 返回实际的长度
     * @post 如果(m_position + len) > m_capacity 则 m_capacity扩容N个节点以容纳len长度
     */
    uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);

    /**
     * @brief 返回数据的长度
     */
    size_t getSize() const { return m_size;}
private:
    
    
     //扩容ByteArray,使其可以容纳size个数据(如果原本可以可以容纳,则不扩容)
    void addCapacity(size_t size);
     // 获取当前的可写入容量
    size_t getCapacity() const { return m_capacity - m_position;}

private:
    /// 内存块的大小
    size_t m_baseSize;
    /// 当前操作位置
    size_t m_position;
    /// 当前的总容量
    size_t m_capacity;
    /// 当前数据的大小
    size_t m_size;
    /// 字节序,默认大端
    int8_t m_endian;
    /// 第一个内存块指针
    Node* m_root;
    /// 当前操作的内存块指针
    Node* m_cur;
};

}

#endif
