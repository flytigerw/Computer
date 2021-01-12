#ifndef SYLAR_LOG_H
#define SYLAR_LOG_H 

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstdarg>
#include <map>
#include "singleton.h"
#include "util.h"
#include "thread.h"




//添加一些marcro，--> 更加方便的使用

//返回流，接受内容，输出指定level的日志
#define  SYLAR_LOG_LEVEL(logger,level) \
         if(logger->getLevel() <= level) \
          sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent \
                                      (logger,level,__FILE__,__LINE__,0,sylar::GetThreadId(),\
                                        sylar::GetFiberId(),time(0),sylar::Thread::GetName())))\
          .getSS()

#define  SYLAR_LOG_DEBUG(logger)  SYLAR_LOG_LEVEL(logger,sylar::LogLevel::DEBUG)
#define  SYLAR_LOG_INFO(logger)   SYLAR_LOG_LEVEL(logger,sylar::LogLevel::INFO)
#define  SYLAR_LOG_WARN(logger)   SYLAR_LOG_LEVEL(logger,sylar::LogLevel::WARN)
#define  SYLAR_LOG_ERROR(logger)  SYLAR_LOG_LEVEL(logger,sylar::LogLevel::ERROR)
#define  SYLAR_LOG_FATAL(logger)  SYLAR_LOG_LEVEL(logger,sylar::LogLevel::FATAL)


//用于自定义format
#define SYLAR_LOG_FMT_LEVEL(logger,level,fmt,...) \
          if(logger->getLevel() <= level)\
           sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent \
                                      (logger,level,__FILE__,__LINE__,0,sylar::GetThreadId(),\
                                        sylar::GetFiberId(),time(0),sylar::Thread::GetName())))\
          .getEvent()->format(fmt,__VA_ARGS__)

#define SYLAR_LOG_FMT_DEBUG(logger,fmt,...) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::DEBUG,fmt,__VA_ARGS__)
#define SYLAR_LOG_FMT_INFO(logger,fmt,...)  SYLAR_LOG_LEVEL(logger,sylar::LogLevel::INFO,fmt,__VA_ARGS__)
#define SYLAR_LOG_FMT_WARN(logger,fmt,...)  SYLAR_LOG_LEVEL(logger,sylar::LogLevel::WARN,fmt,__VA_ARGS__)
#define SYLAR_LOG_FMT_FATAL(logger,fmt,...) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::FATAL,fmt,__VA_ARGS__)
#define SYLAR_LOG_FMT_ERROR(logger,fmt,...) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::ERROR,fmt,__VA_ARGS__)

//从LoggerManager中获取root Logger
#define SYLAR_LOG_ROOT() sylar::LoggerMgr::GetInstance()->getRoot()
//获取指定logger
#define SYLAR_LOG_LOGGER(name) sylar::LoggerMgr::GetInstance()->getLogger(name)






namespace sylar{
  class Logger;
  class LoggerManager;
    //日志级别
  class LogLevel{
    public:
      enum Level{
          UNKNOWN = 0,
          DEBUG = 1,
          INFO = 2,
          WARN = 3,
          ERROR = 4,
          FATAL = 5
      };
      //将level转为对应的string
      static const char* ToString(LogLevel::Level);
      static LogLevel::Level FromString(const std::string&);
  };


  //要打出Log的时候，将其定义为Event，所需要log内容都在其中
  class LogEvent{
    public:
      typedef std::shared_ptr<LogEvent> ptr;

      LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,const char* file,
              int32_t line,uint32_t elapse,uint32_t thread_id,uint32_t fiber_id,uint64_t time,const std::string& thread_name)
              :m_file(file),
               m_line(line),
               m_elapse(elapse),
               m_threadId(thread_id),
               m_fiberId(fiber_id),
               m_time(time),
               m_logger(logger),
               m_level(level),
               m_thread_name(thread_name){
               }


      const char* getFile() const {return m_file;}
      int32_t getLine() const {return m_line;}
      uint32_t getElapse() const {return m_elapse;}
      uint32_t getThreadId() const {return m_threadId;}
      const std::string getThreadName() const {return m_thread_name;}
      uint32_t getFiberId() const {return m_fiberId;}
      uint64_t getTime() const {return m_time;}
      std::string getContent() const {return m_ss.str();}
      std::stringstream& getSS(){return m_ss;}
      std::shared_ptr<Logger> getLogger() const{return m_logger;}
      LogLevel::Level getLevel() const{return m_level;}
      void format(const char* fmt,va_list al);
      void format(const char* fmt,...);

      
    private:
      //log文件所在地
      const char* m_file = nullptr;
      //行号
      int32_t m_line = 0;
      //程序启动开始到现在的毫秒数
      uint32_t m_elapse = 0;   
      //线程ID
      uint32_t m_threadId= 0;
      //协程ID
      uint32_t m_fiberId= 0;
      uint64_t m_time = 0;
      //log内容
      std::stringstream m_ss;
      //要将自己写入哪个logger中
      std::shared_ptr<Logger> m_logger;
      LogLevel::Level m_level;
      std::string m_thread_name;
  };
  
  class LogEventWrap{
    public:
      LogEventWrap(LogEvent::ptr e): m_event(e){}
      ~LogEventWrap();
      std::stringstream& getSS(){
        return m_event->getSS();
      }
    private:
      LogEvent::ptr  m_event;
  };

  class LogFormatter{
    public:
      typedef std::shared_ptr<LogFormatter> ptr;

      LogFormatter(const std::string& s) : m_pattern(s){init();}
      //将event的内容进行格式化，再进行输出
      //将logger传给formatter,让formatter可以输出logger的名称
      std::string format(std::shared_ptr<Logger>  logger,LogLevel::Level ,LogEvent::ptr event);

      bool hasError() const{return m_error;}
      //进行pattern的解析
      const std::string getPattern()const {return m_pattern;}
      void init();

    public:
      //格式化的格式 ----> 格式中含有多种元素 ----> 每个元素用一个item表示
      //基类
      class FormatItem{
        public:
          typedef std::shared_ptr<FormatItem> ptr;
          virtual ~FormatItem(){}
          virtual void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level,LogEvent::ptr event) = 0;
      };

    private:
      std::vector<FormatItem::ptr> m_items;
      std::string m_pattern;
      bool m_error = false;
  };

  //日志输出类
  class LogAppender{
    friend class Logger;
      public:
      typedef SpinMutex MutexType;
        typedef std::shared_ptr<LogAppender> ptr;
        virtual ~LogAppender() {}
      //打印log时需要给出level,且由event驱动
        virtual void log(std::shared_ptr<Logger> logger,LogLevel::Level leve,LogEvent::ptr event) = 0;
        virtual std::string toYamlString() = 0;
        
        void setFormatter(LogFormatter::ptr val);
        LogFormatter::ptr getFormatter() const;
        LogLevel::Level getLevel() const {return m_level;}
        void setLevel(LogLevel::Level level){m_level = level;}
      protected:
        LogLevel::Level m_level;
        //输出格式后的日志
        LogFormatter::ptr m_formatter;
        //当appender自己没有formatter时就会用到logger的formatter ----> 二者需要区分
        bool has_formatter = false;
        //写入时可能会引起race condition 
        mutable MutexType m_mutex;
  };


  //日志器:包含名字，日志输出集合，日志level ---> 要满足日志级别
  class Logger: public std::enable_shared_from_this<Logger> {
    friend class LoggerManager;
    public:
      typedef Mutex MutexType;
      typedef std::shared_ptr<Logger> ptr;
      Logger(const std::string& name = "root"):m_name(name),m_level(LogLevel::DEBUG){
       m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
     }
      //输出满足level的日志
      //将自身向下传到formatter，让其可以输出logger的名称
      void log(LogLevel::Level level,LogEvent::ptr event);

      //输出不同level的日志
      void debug(LogEvent::ptr event);
      void info(LogEvent::ptr event);
      void warn(LogEvent::ptr event);
      void fatal(LogEvent::ptr event);
      void error(LogEvent::ptr event);

      void addAppender(LogAppender::ptr appender);
      void delAppender(LogAppender::ptr appender);
      void clearAppenders();

      void setFormatter(LogFormatter::ptr);
      void setFormatter(const std::string&);
      LogFormatter::ptr getFormatter()const;

      std::string toYamlString();
      LogLevel::Level getLevel() const {return m_level;}
      void setLevel(LogLevel::Level val) {m_level  = val;}
      const std::string getName() const{return m_name;}

    private:
      std::string m_name;
      std::list< LogAppender::ptr> m_appenders;
      LogLevel::Level m_level;
      LogFormatter::ptr  m_formatter;

      //一般会定义一个全局的root logger 
      //对于自定义的logger,若其没有appender,则通过root looger的appender进行输出
      Logger::ptr m_root;
      //使用MutexType保护数据成员
      mutable MutexType m_mutex;
  };


  //输出到控制台的appender
  class StdoutLogAppender : public LogAppender{
    friend class Logger;
    public:
      typedef std::shared_ptr<StdoutLogAppender> ptr;
      void log(std::shared_ptr<Logger>,LogLevel::Level,LogEvent::ptr event) override;
      std::string toYamlString() override;
  };

  //输出到文件的appender
  class FileLogAppender : public LogAppender{
    friend class Logger;
    public:
      typedef std::shared_ptr<FileLogAppender> ptr;
      std::string toYamlString() override;

      FileLogAppender(const std::string& name);
      void log(std::shared_ptr<Logger> logger,LogLevel::Level,LogEvent::ptr event) override;

      //重新打开文件，返回是否成功打开
      bool reopen();
    private:
      //输出的文件名
      std::string m_filename;
      //利用文件流进行输出
      std::ofstream m_filestream;
  };
  
  //每次都手动创建logger太过于麻烦 ----> 用mananger管理一批logger
  //根据name快速取一个logger
  class LoggerManager{
    public:
      typedef SpinMutex MutexType;
      LoggerManager();
      Logger::ptr getLogger(const std::string& name);
      Logger::ptr getRoot()const{return m_root;}
      void init();
      std::string toYamlString();
    private:
      std::map<std::string,Logger::ptr> m_loggers;
      Logger::ptr m_root;
      mutable MutexType m_mutex;
  };

  typedef sylar::SingletonPtr<LoggerManager> LoggerMgr;
}

#endif 
