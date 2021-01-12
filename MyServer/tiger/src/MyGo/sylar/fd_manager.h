#ifndef MANAGER_H
#define MANAGER_H 


#include <memory>
#include <vector>
#include "thread.h"
#include "iomanager.h"
#include "singleton.h"
namespace sylar{

  //记录fd的context ---> 状态，属性等 ---> 用户层面
  class FdCtx:public std::enable_shared_from_this<FdCtx>{

    public:
      typedef std::shared_ptr<FdCtx> ptr;
      FdCtx(int fd);
      ~FdCtx();

      bool init();

      bool isInit() const{return m_isInit;}
      bool isSocket() const{return m_isSocket;}
      bool isClosed() const{return m_isClosed;}
      bool isUserNonblock() const{return m_isUserNonBlock;}
      bool isSysNonblock() const{return m_isSysNonBlock;}
      //读或者写是否超时
      uint64_t getTimeout(int type) const;

      void setUserNonblock(bool v){m_isUserNonBlock = v;}
      void setSysNonblock(bool v){m_isSysNonBlock = v;} 
      //Type:读或写
      void setTimeout(int type,uint64_t v);

    private:
        bool m_isInit = 1;
        bool m_isSocket = 1;
        bool m_isSysNonBlock =1;
        bool m_isUserNonBlock =1;
        bool m_isClosed =1;
        int m_fd;
        uint64_t m_recvTimeout;
        uint64_t m_sendTimeout;

        IOManager* m_iom;
  };

  class FdManager{
     public:
       typedef RWMutex RWMutexType;
       FdManager();
       
       //获取fd相关量的FdCtx,若fd_ctx不存在，则根据auto_create进行创建
       FdCtx::ptr get(int fd,bool auto_create = false);
       void del(int fd);

     private:
       RWMutexType m_mutex;
       std::vector<FdCtx::ptr> m_datas;
  };
  
  typedef Singleton<FdManager> FdMgr;
}

#endif
