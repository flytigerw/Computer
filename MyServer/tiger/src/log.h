#ifndef MY_LOGGER_H
#define MY_LOGGER_H 

#include <string>
#include <memory>
#include <sstream>
#include <fstream>
#include <stdint.h>
#include <map>
#include <vector>
#include "util.h"
#include "singleton.h"
#include "thread.h"

//编写一些macro，使整个日志系统更加好用
#define TIGER_LOG_LOG(g_logger,level)\
  if(true)\
  tiger::Wrapper(g_logger,tiger::LogEvent::ptr(new tiger::LogEvent(__FILE__,__LINE__,tiger::Thread::GetCurThreadName(),time(0),tiger::GetThreadId(),tiger::GetFiberId())),level).set_content()


#define TIGER_LOG_DEBUG(g_logger)   TIGER_LOG_LOG(g_logger,tiger::LogLevel::DEBUG)
#define TIGER_LOG_ERROR(g_logger)   TIGER_LOG_LOG(g_logger,tiger::LogLevel::ERROR)
#define TIGER_LOG_INFO(g_logger)    TIGER_LOG_LOG(g_logger,tiger::LogLevel::INFO)
#define TIGER_LOG_FATAL(g_logger)   TIGER_LOG_LOG(g_logger,tiger::LogLevel::FATAL)
#define TIGER_LOG_UNKNOWN(g_logger) TIGER_LOG_LOG(g_logger,tiger::LogLevel::UNKNOWN)

//通过macro,迅速通过LoggerMgr创建Logger
#define TIGER_LOG_LOGGER(name) \
  tiger::LoggerMgr::GetInstance()->LookUp(name)
#define TIGER_LOG_ROOT \
  tiger::LoggerMgr::GetInstance()->get_root_logger()


namespace tiger{

  class Logger;
  

  class LogLevel{
    public:
    enum Level{
      UNKNOWN = 0,
      DEBUG = 1,
      INFO =2,
      WARN = 3,
      ERROR = 4,
      FATAL = 5
    };
    static const std::string LevelToStr(LogLevel::Level);
  };

  class LogEvent{
    public:
      //采用智能指针托管资源
      typedef std::shared_ptr<LogEvent> ptr;
      LogEvent(const char* file,int32_t line,const std::string& thread_name,time_t time,uint32_t thread_id,uint32_t fiber_id):
        m_file(file),
        m_line(line),
        m_thread_name(thread_name),
        m_time(time),
        m_thread_id(thread_id),
        m_fiber_id(fiber_id){}

      //get方法:供FormatItem调用
      const char* get_file() const{return m_file;}
      int32_t get_line() const{return m_line;}
      const std::string& get_thread_name() const{return m_thread_name;}
      const std::string& get_logger_name() const{return m_logger_name;}

      uint32_t get_thread_id() const{return m_thread_id;}
      uint32_t get_fiber_id() const{return m_fiber_id;}
      time_t get_time() const{return m_time;}
      std::string get_content() const {return m_content.str();}
      const std::string get_level() const{return LogLevel::LevelToStr(m_level);}

      //使用流来设置content
      std::stringstream& set_content(){return m_content;}
      void set_logger_name(const std::string& name){m_logger_name = name;}
      void set_logger_level(LogLevel::Level level){m_level = level;}

    private:
      //日志所在文件名，行号
      //打印日志的时间
      //由那个线程和协程,logger打印
      //日志内容
      const char*  m_file;          //使用const char*的原因 ---> macro __FILE__返回m_file
      int32_t m_line;
      std::string m_thread_name;
      std::string m_logger_name ;
      LogLevel::Level m_level;
      //logger_name和log_level并不在初始化时进行设置，而是在logger将log_event传给LogAppender时借用logger的
      time_t m_time;
      uint32_t m_thread_id;
      uint32_t m_fiber_id;
      std::stringstream m_content;
  };


  class LogFormatter{
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
      LogFormatter(const std::string& pattern 
                  ="%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n");

      //对外接口
      std::string format(LogEvent::ptr event);
      const std::string& get_pattern() const{return m_pattern;}
      void set_pattern(const std::string& p){m_pattern = p;}
                   
    public:
      class FormatItem{
        public:
          typedef std::shared_ptr<FormatItem> ptr;
          virtual ~FormatItem(){}
          virtual void format(std::stringstream& os,LogEvent::ptr event) = 0;
      };
    private:
      std::string m_pattern;
      std::vector<FormatItem::ptr> m_items;
      bool m_error = false;
      void parse_pattern();
  };

  
  class LogAppender{
    public:
      typedef std::shared_ptr<LogAppender> ptr;
      LogAppender(LogFormatter::ptr p=nullptr);
  
      virtual ~LogAppender(){}
      virtual void log(LogEvent::ptr) = 0;
      virtual std::string to_yaml_string() =  0;
      void set_formatter(LogFormatter::ptr f){m_formatter = f;}
      void set_formatter(const std::string& pattern){m_formatter.reset(new LogFormatter(pattern)) ;}
    protected:
      LogFormatter::ptr m_formatter;
  };

  class StdoutLogAppender : public LogAppender{
    public:
      StdoutLogAppender(LogFormatter::ptr p=nullptr):LogAppender(p){}
      virtual void log(LogEvent::ptr) override;
      std::string to_yaml_string() override;
  };

  class FileLogAppender : public LogAppender{
    public:
      FileLogAppender(const std::string& file,LogFormatter::ptr p=nullptr);
      virtual void log(LogEvent::ptr) override;
      void open_file();
      std::string to_yaml_string() override;
    private:
      std::string m_filename;
      std::ofstream m_file_stream;
      int64_t m_lastTime = 0;           //定期对文件进行close
  };

  class Logger{
    public:
      typedef std::shared_ptr<Logger> ptr;
      Logger(const std::string& name):m_name(name){m_formatter.reset(new LogFormatter);}

      void log(LogEvent::ptr event,LogLevel::Level level);
      void debug(LogEvent::ptr event);
      void info(LogEvent::ptr event);
      void error(LogEvent::ptr event);
      void warn(LogEvent::ptr event);
      void fatal(LogEvent::ptr event);

      //m_formatter相关操作
      LogFormatter::ptr get_formatter() const {return m_formatter;}
      void set_formatter(const std::string& pattern) ;
      void set_formatter(LogFormatter::ptr) ;
      
      //m_appenders操作
      void add_appender(LogAppender::ptr appender){if(appender != nullptr) m_appenders.push_back(appender);}
      //用户可获取m_appenders,修改其中appender的formatter
      std::vector<LogAppender::ptr>& get_appenders() {return m_appenders;}
      const std::vector<LogAppender::ptr>& get_appenders() const {return m_appenders;}
      void del_appender(LogAppender::ptr);
      void clear_appenders();

      //让m_appenders的formatter和logger的一致
      void sync_fomatter();

      std::string to_yaml_string();
    private:
      std::vector<LogAppender::ptr> m_appenders;
      LogFormatter::ptr m_formatter ;
      std::string m_name;
  };

  class LoggerManager{
    public:
      LoggerManager();
      //使用找不到就创建语义
     Logger::ptr LookUp(const std::string& name);
     Logger::ptr get_root_logger() const {return m_root_logger;}

     void add_logger(const std::string& name,Logger::ptr);
     void add_logger(const std::string& name);
     void del_logger(const std::string& name);

     //输出yaml格式的string ----> 便于观看
     std::string to_yaml_string();
    
    private:
     std::map<std::string,Logger::ptr> m_loggers;
     Logger::ptr m_root_logger;
  };
  typedef SharedSingleton<LoggerManager> LoggerMgr;

  class Wrapper{
    public:
      Wrapper(Logger::ptr logger,LogEvent::ptr event,LogLevel::Level level)
        :m_logger(logger),m_event(event),m_level(level){}

      ~Wrapper(){
        m_logger->log(m_event,m_level);
      }
      std::stringstream& set_content(){return  m_event->set_content();}

    private:
      Logger::ptr m_logger;
      LogEvent::ptr m_event;
      LogLevel::Level m_level;
  };

}

#endif
