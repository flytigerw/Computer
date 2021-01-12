#pragma once
#include <unordered_map>
#include "../common/lock.h"
#include "fdcontext.h"

namespace tiger{



  //用于为fd添加NON_BLOCK flag
struct NonBlockingGuard {
    explicit NonBlockingGuard(const FdContext::ptr& fd_ctx);
    ~NonBlockingGuard();

    FdContext::ptr m_fd_ctx;
    bool is_nonblocking;
};



  //管理一堆fdcontext

class FdCtxManager{


  public:
    //fd 槽位
    struct FdSlot{
      LFLock m_lock;
      FdContext::ptr m_fd_ctx;
    };
    typedef std::shared_ptr<FdSlot> FdSlotPtr;

    static const int StaticFdSize = 128;
    static const int BucketShift = 10; //移位数
    static const int BucketCount = (1 << BucketShift) - 1; //1023个

  private:
    //建立FdContext的hash表
    typedef std::unordered_map<int,FdSlotPtr> Slots;
    //二重散列:
    //由于fd可能很大 ---> 超过1024
    //于是将fd进行高位截肢 ---> 只要低9位 ---> fd & BucketCount
    //比如:1025和1都位于第一个桶  ---> 一重散列
    //在桶内再根据fd进行散列      ---> 二重散列
    Slots m_buckets[BucketCount+1];  //1024个
    std::mutex bucket_mtxs[BucketCount+1];

  public:
    static FdCtxManager& GetInstance();
    
    FdContext::ptr get_fd_context(int fd);

    //有些socket的close行为hook不到, 创建时如果有旧的context直接close掉即可.
    void create(int fd, FdType fd_type, bool isNonBlocking = false,
            SocketAttribute sockAttr = SocketAttribute());
    // 在syscall之前调用
    void close(int fd);
    // 在syscall之后调用
    void dup(int from, int to);
    
  private:
    FdSlotPtr get_slot(int fd);
    void insert(int fd,FdContext::ptr fd_ctx);
};

}
