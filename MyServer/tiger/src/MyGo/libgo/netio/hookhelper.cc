


#include "hookhelper.h"


namespace tiger{


  //构造时保证fd为non_block
  NonBlockingGuard::NonBlockingGuard(const FdContext::ptr& fd_ctx) : m_fd_ctx(fd_ctx){
    is_nonblocking = fd_ctx->is_nonblocking();
    if(!is_nonblocking)
      fd_ctx->set_non_blocking_really(true);
  }

  NonBlockingGuard::~NonBlockingGuard(){
    if(!is_nonblocking)
      m_fd_ctx->set_non_blocking_really(false);
  }




  FdCtxManager& FdCtxManager::GetInstance(){
    static FdCtxManager obj;
    return obj;
  }


  FdContext::ptr FdCtxManager::get_fd_context(int fd){
    FdSlotPtr slot = get_slot(fd);  
    if(!slot)
      return FdContext::ptr();
    std::unique_lock<LFLock> lock_guard(slot->m_lock);
    //生成智能指针传出去
    FdContext::ptr fd_ctx(slot->m_fd_ctx);
    return fd_ctx;
  }

    void FdCtxManager::create(int fd, FdType fd_type, bool isNonBlocking ,
        SocketAttribute sockAttr ){
      //new一个，插入到表中
      FdContext::ptr ctx(new FdContext(fd, fd_type, isNonBlocking, sockAttr));
      insert(fd, ctx);
    }

    FdCtxManager::FdSlotPtr get_slot(int fd);
    void FdCtxManager::insert(int fd,FdContext::ptr fd_ctx){

      //一重散列
      int bucket_idx = fd & BucketCount;
      std::unique_lock<std::mutex> lock_guard(bucket_mtxs[bucket_idx]);
      auto& bucket = m_buckets[bucket_idx];
      //二重散列
      FdSlotPtr& slot = bucket[fd]; 
      if (!slot) 
        slot.reset(new FdSlot);  //新建一个
      lock_guard.unlock();

      //fd_ctx已经存在  ---> fd的属性已更新
      //close 旧的fd_ctx,插入新的fd_ctx
      FdContext::ptr closed_fd_ctx;
      std::unique_lock<LFLock> lock_guard2(slot->m_lock);
      closed_fd_ctx.swap(slot->m_fd_ctx);   //为什么采用swap? --->  因为要在调用close以后才能析构  
      slot->m_fd_ctx = fd_ctx;  //新的fd_ctx
      lock_guard2.unlock();
      if(closed_fd_ctx) 
        closed_fd_ctx->close();
    }

    //close fd对应的fdcontext
    void FdCtxManager::close(int fd){
        FdSlotPtr slot = get_slot(fd);
        if(!slot)
          return;
        FdContext::ptr fd_ctx;
        std::unique_lock<LFLock> lock_guard2(slot->m_lock);
        fd_ctx.swap(slot->m_fd_ctx); 
        lock_guard2.unlock();

        if(fd_ctx)
          fd_ctx->close();
        
    }

    //一重索引
    FdCtxManager::FdSlotPtr FdCtxManager::get_slot(int fd){
      int bucket_idx = fd & BucketCount;
      std::unique_lock<std::mutex> lock_guard(bucket_mtxs[bucket_idx]);
      auto& bucket = m_buckets[bucket_idx];
      auto it = bucket.find(fd);
      return bucket.find(fd) == bucket.end() ? 
             FdSlotPtr() :
             it->second;
    }

    //将from fd的属性拷贝一份，以to fd插入
    void FdCtxManager::dup(int from, int to){
      FdContext::ptr fd_ctx = get_fd_context(from);
      if(!fd_ctx)
        return;
      //clonne:属性拷贝
      insert(to,fd_ctx->clone(to));
    }

}
