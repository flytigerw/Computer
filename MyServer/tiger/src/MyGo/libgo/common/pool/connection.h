

#pragma once 

#include "../../channel/channel.h"
#include "../clock.h"


namespace tiger{


  //连接池
  //connection:连接类型.比如数据库连接
  template<class Connection>
    class ConnectionPool{

      public:
        typedef std::shared_ptr<Connection> ConnectionPtr;
        typedef std::function<Connection*()> Factory; //创建连接的工厂
        typedef std::function<void(Connection*)> Deleter; //销毁连接
        typedef std::function<bool(Connection*)> CheckAlive;
      private:
        Factory m_factory;
        Deleter m_deleter;
        //最大连接数
        size_t max_connection,max_idle_connection;//最大空闲连接数
        Channel<Connection*> m_channel; //用channel存放连接
        std::atomic<size_t> m_count; //为什么要用原子变量呢

      public:
       explicit ConnectionPool(Factory f, Deleter d = nullptr, size_t max_conn = 0, size_t max_idle = 0)
        : m_factory(f), m_deleter(d), m_count(0),
          max_connection(max_conn),
          max_idle_connection(max_idle == 0 ? max_conn : max_idle),
          m_channel(max_idle){     //channel容量 
        
            if (!m_deleter) {
             m_deleter = [](Connection * ptr){ delete ptr; };
        }
    }

       ~ConnectionPool(){
         Connection* p  = nullptr;
         while(m_channel.try_pop(p));
           //

       }

       //私有辅助函数
      private:
       Connection* create_connection(){
         if(++m_count > max_connection){
           --m_count;
           return nullptr;
         }
         //利用工厂函数创建一个
         return m_factory();
       }

       //销毁一个连接
       void destroy(Connection* connection){
         --m_count;
         m_deleter(connection);
       }
       //将一个连接放回池中
       void put_back(Connection* connection){
         if(!m_channel.try_push(connection))
           destroy(connection);
       }

       ConnectionPtr convert(Connection* conn,CheckAlive check_alive_on_put){
         return ConnectionPtr(conn,
                              [this,check_alive_on_put](ConnectionPtr* p){   //注意闭包中的this
                                   if(check_alive_on_put && !check_alive_on_put(p)){
                                         this->destroy(p);
                                         return nullptr;
                                   }
                            this->put_back(p);
                            });
       }

      public:
       size_t count(){
         return m_count;
       }

       //预先创建一些连接
       //不能超过最大空闲连接数
       void reserve(size_t n){
         for(size_t i=count();i<max_idle_connection;++i){
           Connection c = create_connection();
           if(!c)
             break;
           //将创建好的连接放入到池中
           if(!m_channel.try_push(c)){
              destroy(c);
              break;
           }
         }
       }

       // 获取一个连接
       // 如果池空了并且连接数达到上限, 则会等待
       // 返回的智能指针销毁时, 会自动将连接归还给池
       // 在get和put时要检测连接是否有效
       ConnectionPtr get(CheckAlive check_alive_on_get = nullptr,CheckAlive check_alive_on_put =nullptr ){
         Connection* conn = nullptr;
RETRY:
         if(m_channel.try_pop(conn)){
           if(check_alive_on_get && !check_alive_on_get(conn)){ //检验失败
             destroy(conn);
             conn = nullptr;
             goto RETRY;
           }
           //检验成功,生成智能指针，指定销毁时的任务:将连接归还给池，规划时需要进行检测
           return convert(conn,check_alive_on_put);
         }

         //pop失败 ---> 没有连接，创建一个
         conn = create_connection();
         if(conn)
           return convert(conn,check_alive_on_put);

         //创建失败
         m_channel >> conn;  // ？？？ 
         if(check_alive_on_get && !check_alive_on_get(conn)){ //检验失败
           destroy(conn);
           conn = nullptr;
           goto RETRY;
         }
         return convert(conn,check_alive_on_put);
       }

       //为get添加等待时间，仅在协程中有效
       ConnectionPtr get(FastSteadyClock::duration timeout,
                         CheckAlive check_alive_on_get = nullptr,
                         CheckAlive check_alive_on_put =nullptr ){
         FastSteadyClock::time_point deadline = FastSteadyClock::now() + timeout;
         Connection* conn = nullptr;
RETRY:
         if(m_channel.try_pop(conn)){
           if(check_alive_on_get && !check_alive_on_get(conn)){ //检验失败
             destroy(conn);
             conn = nullptr;
             goto RETRY;
           }
           //检验成功,生成智能指针，指定销毁时的任务:将连接归还给池，规划时需要进行检测
           return convert(conn,check_alive_on_put);
         }

         //pop失败 ---> 没有连接，创建一个
         conn = create_connection();
         if(conn)
           return convert(conn,check_alive_on_put);

         //阻塞一段时间收再pop
         if(m_channel.time_pop(conn,deadline)){
           if(check_alive_on_get && !check_alive_on_get(conn)){ //检验失败
             destroy(conn);
             conn = nullptr;
             goto RETRY;
           }
           return convert(conn,check_alive_on_put);
       }
        return ConnectionPtr();
     }
    };

}
