
#include "fd_manager.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include "hook.h"

namespace sylar{

  FdCtx::FdCtx(int fd)
     :m_isInit(false)
     ,m_isSocket(false)
     ,m_isSysNonBlock(false)
     ,m_isUserNonBlock(false)
     ,m_isClosed(false)
     ,m_fd(fd)
     ,m_recvTimeout(-1)
     ,m_sendTimeout(-1) {
     init();
   }

  FdCtx::~FdCtx() {
  }


  bool FdCtx::init(){ //针对其他属性进行初始化
    if(m_isInit)
      return true;
    m_recvTimeout = -1;
    m_sendTimeout = -1;
    
    struct stat fd_stat; //获取文件属性
    if(fstat(m_fd,&fd_stat) == -1){
      m_isInit = false;
      m_isSocket = false;
    }else{
      m_isInit = true;
      //判断文件是否为socket
      m_isSocket = S_ISSOCK(fd_stat.st_mode);
    }

    if(m_isSocket){ //设置socket文件的相关属性:O_NONBLOCK
      int flags = fcntl_f(m_fd,F_GETFL,0);
      if(!(flags & O_NONBLOCK)){ //自身不含有O_NONBLOCK
        fcntl_f(m_fd,F_SETFL,flags | O_NONBLOCK);
        m_isSysNonBlock = true;
      }else 
        m_isSysNonBlock = false;
    }
    m_isUserNonBlock = false;
    m_isClosed = false;
    return m_isInit;
  }

  
  void FdCtx::setTimeout(int type, uint64_t v) {
      if(type == SO_RCVTIMEO)  //设置socket接受数据的超时标志
          m_recvTimeout = v;
       else 
          m_sendTimeout = v;
  }

  uint64_t FdCtx::getTimeout(int type) const {
      if(type == SO_RCVTIMEO) 
          return m_recvTimeout;
       else 
          return m_sendTimeout;
  }
  FdManager::FdManager() {
      m_datas.resize(64);
  }

  FdCtx::ptr FdManager::get(int fd,bool auto_create){
    if(fd == -1)
      return  nullptr;
    //fd作为索引，查看其是否存在
    //查询操作
    RWMutexType::ReadLockGuard lock_guard(m_mutex);
    if((int)m_datas.size() <= fd){
      if(auto_create == false)
        return nullptr; 
    }else{
      if(m_datas[fd] || !auto_create)
        return m_datas[fd];
    }
    lock_guard.unlock();

    //运行到此，说明需要create fd 
    RWMutexType::WriteLockGuard lock_guard2(m_mutex);
    FdCtx::ptr ctx(new FdCtx(fd));
    //考虑是否扩容
    if(fd >= (int)m_datas.size())
      m_datas.resize(fd*1.5);
    m_datas[fd] = ctx;
    return ctx;
  }

  void FdManager::del(int fd){
    RWMutexType::ReadLockGuard lock_guard(m_mutex);
    if((int)m_datas.size() <= fd)
      return; 
    //释放该shared_ptr即可 --> 释放资源
    m_datas[fd].reset();
  }

}
