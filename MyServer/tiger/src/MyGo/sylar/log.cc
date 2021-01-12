#include <map>
#include <functional>
#include <time.h>
#include <string.h>
#include "config.h"
#include "log.h"

#include "log.h"
namespace sylar{
  const char* LogLevel::ToString(LogLevel::Level level){
    switch (level){
      //利用macro处理相同部分
#define XX(name) \
      case LogLevel::Level::name:\
        return #name; \
        break;

      XX(DEBUG);
      XX(INFO);
      XX(WARN);
      XX(ERROR);
      XX(FATAL);
#undef XX

      default:
        return "UNKNOWN";
    }
  } 


  LogLevel::Level LogLevel::FromString(const std::string& str){
#define XX(name,s)\
    if(str == #s) {\
      return LogLevel::name; \
    }

    XX(DEBUG,debug);
    XX(WARN,warn);
    XX(INFO,info);
    XX(ERROR,error);  
    XX(FATAL,fatal);

    XX(DEBUG,DEBUG);
    XX(WARN,WARN);
    XX(INFO,INFO);
    XX(ERROR,ERROR);  
    XX(FATAL,FATAL);
#undef XX
    return LogLevel::UNKNOWN;
  }

//--------------------------------------------------Logger-------------------------------------------------------
  void Logger::log(LogLevel::Level level,LogEvent::ptr event){
    if(level >= m_level){ //用相应的appender进行输出
      MutexType::LockGuard lock_guard(m_mutex);
      if(!m_appenders.empty()){
        for(auto& i : m_appenders)
          i->log(shared_from_this(),level,event);
      }else m_root->log(level,event);        //借助root logger进行log
    }
  }

  void  Logger::clearAppenders(){
    m_appenders.clear();
  }
  void Logger:: debug(LogEvent::ptr event){
         log(LogLevel::DEBUG,event);
  }
  void Logger:: info(LogEvent::ptr event){
          log(LogLevel::INFO,event);
  }
  void Logger:: warn(LogEvent::ptr event){
          log(LogLevel::WARN,event);

  }
  void Logger:: fatal(LogEvent::ptr event){
          log(LogLevel::FATAL,event);

  }
  void Logger:: error(LogEvent::ptr event){
          log(LogLevel::ERROR,event);

  }
  void Logger::setFormatter(LogFormatter::ptr val){
          MutexType::LockGuard lock_guard(m_mutex);
          m_formatter  = val;
          for(auto& i : m_appenders){
            LogAppender::MutexType::LockGuard lock_guard(i->m_mutex);
            if(!i->has_formatter)
              i->m_formatter = m_formatter;
          }
  }

  void Logger::setFormatter(const std::string& str){
          
      //用str创建新的formatter
    LogFormatter::ptr new_val(new sylar::LogFormatter(str));
    if(new_val->hasError()){  //error 检查
      std::cout << "Logger setFormatter name=" << m_name 
                << "value = " << str << "invalid formatter" << std::endl;
      
      return;
     }
    setFormatter(new_val);
  }

 std::string Logger::toYamlString(){
    MutexType::LockGuard lock_guard(m_mutex);
    YAML::Node node;
    node["name"] = m_name;
    if(m_level != LogLevel::UNKNOWN) {
       node["level"] = LogLevel::ToString(m_level);
    }
    if(!m_formatter->getPattern().empty())
      node["formatter"] = m_formatter->getPattern();
    for(auto& i : m_appenders)
      node["appenders"].push_back(YAML::Load(i->toYamlString()));
    std::stringstream ss;
    ss << node;
    return ss.str();
  }

  LogFormatter::ptr Logger::getFormatter() const{
    MutexType::LockGuard lock_guard(m_mutex);
    return m_formatter;
  }
  void Logger:: addAppender(LogAppender::ptr appender){
    //把自己的formatter传给appender
    MutexType::LockGuard lock_guard(m_mutex);
    if(!appender->getFormatter())
      LogAppender::MutexType::LockGuard lock_guard(appender->m_mutex);
      appender->m_formatter = m_formatter;      //不能调用formatter的setFormatter ----> 会设置has_foramatter flag
    m_appenders.push_back(appender);
  }
  void Logger:: delAppender(LogAppender::ptr appender){
    MutexType::LockGuard lock_guard(m_mutex);
    for(auto it= m_appenders.begin(); it != m_appenders.end(); it++){
      if(*it == appender){
        m_appenders.erase(it);
        break;
      }
    } 
  }
   
  

//-----------------------------------------------LogAppender-----------------------------------------------
  void LogAppender::setFormatter(LogFormatter::ptr val){
        //数据修改与读入需要加锁
        //formatter为复杂类型，本身可能还包含多种其他类型，若设置时被打断 ----> 不一致
         MutexType::LockGuard lock_guard(m_mutex);
         m_formatter = val;
         has_formatter = m_formatter ? true : false;
  }

  LogFormatter::ptr LogAppender::getFormatter() const {
    MutexType::LockGuard lock_guard(m_mutex);
    return m_formatter;
  }

  void StdoutLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) {
    //同样只输出特定level的日志
    if(level >= m_level){
      MutexType::LockGuard lock_guard(m_mutex);
      std::cout << m_formatter->format(logger,level,event);
    }
  }
  std::string  StdoutLogAppender::toYamlString(){
    MutexType::LockGuard lock_guard(m_mutex);
      YAML::Node node;
      node["type"] = "StdoutLogAppender";
      if(m_level != LogLevel::UNKNOWN) {
        node["level"] = LogLevel::ToString(m_level);
      }
      if(has_formatter && !m_formatter->getPattern().empty())
        node["formatter"] = m_formatter->getPattern();
      std::stringstream ss;
      ss << node;
      return ss.str();
  }
  std::string FileLogAppender::toYamlString(){
    MutexType::LockGuard lock_guard(m_mutex);
      YAML::Node node;
      node["type"] = "FileLogAppender";
      node["file"] = m_filename;
      if(m_level != LogLevel::UNKNOWN) {
         node["level"] = LogLevel::ToString(m_level);
      }
      //反序列为yaml时，若为自己配置formatter，则进行输出。若有logger设置，则不输出
      if(has_formatter && !m_formatter->getPattern().empty())
        node["formatter"] = m_formatter->getPattern();
      std::stringstream ss;
      ss << node;
      return ss.str();
  }
  void FileLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) {

    if(level >= m_level){
      MutexType::LockGuard lock_guard(m_mutex);
      m_filestream << m_formatter->format(logger,level,event);
    }
  }
  bool FileLogAppender::reopen() {
    MutexType::LockGuard lock_guard(m_mutex);
      if(m_filestream)
        m_filestream.close();
      m_filestream.open(m_filename);
      return m_filestream;
  }
  
  FileLogAppender:: FileLogAppender(const std::string& name) : m_filename(name){reopen();}

//-------------------------------------------LogEvent And Wrap --------------------------------
  void LogEvent::format(const char* fmt,...){
    va_list al;
    va_start(al,fmt);
    format(fmt,al);
    va_end(al);
  }
  void LogEvent::format(const char* fmt,va_list al){
    char* buf = nullptr;
    //valist不知道大小和类型，所以通过缓冲区作为中转站
    int len = vasprintf(&buf,fmt,al);
    if(len != -1){
        m_ss << std::string(buf,len);
        free(buf);
    }
  }
  //析构时进行日志打印?
  LogEventWrap::~LogEventWrap(){
     m_event->getLogger()->log(m_event->getLevel(),m_event); 
  }

//-------------------------------------------LogFormatter ------------------------------------------------
  std::string LogFormatter::format(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event){
    std::stringstream ss;
    //调用各个item的解析，将解析结果存放在string流中
    for(auto& it:m_items)
      it->format(ss,logger,level,event);
    return ss.str();
  }
  class MessageFormatItem: public LogFormatter::FormatItem{
    public:
      MessageFormatItem(const std::string& s =""){}
      void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level,LogEvent::ptr event ) override{
          os << event->getContent();
      }
  };

  class LevelFormatItem: public LogFormatter::FormatItem{
    public:
      LevelFormatItem(const std::string& s =""){}
      void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event ) override{
          os << LogLevel::ToString(level);
      }
  };

  class TableFormatItem: public LogFormatter::FormatItem{
    public:
      TableFormatItem(const std::string& s =""){}
      void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event ) override{
          os << "\t";
      }
  };

  class ElapseFormatItem: public LogFormatter::FormatItem{
    public:
      ElapseFormatItem(const std::string& s =""){}
      void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event ) override{
          os << event->getElapse();
      }
  };

  class NameFormatItem: public LogFormatter::FormatItem{
    public:
      NameFormatItem(const std::string& s =""){}
      void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event ) override{
        //event的Logger是最原始的logger 
        os << event->getLogger()->getName();
      }
  };

  class ThreadIdFormatItem: public LogFormatter::FormatItem{
    public:
      ThreadIdFormatItem(const std::string& s =""){}
      void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event ) override{
          os << event->getThreadId();
      }
  };
  class ThreadNameFormatItem: public LogFormatter::FormatItem{
    public:
      ThreadNameFormatItem(const std::string& s =""){}
      void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event ) override{
          os << event->getThreadName();
      }
  };

  class FiberIdFormatItem: public LogFormatter::FormatItem{
    public:
      FiberIdFormatItem(const std::string& s =""){}
      void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event ) override{
          os << event->getFiberId();
      }
  };

  class DateFormatItem: public LogFormatter::FormatItem{
    public:
      DateFormatItem(const std::string& fmt = "%Y-%m-%d %H:%M:%S") :m_format(fmt){
        if(m_format.empty())
          m_format = "%Y-%m-%d %H:%M:%S";  //=------------important
      }
     void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event ) override{
       struct tm t;
       time_t time = event->getTime();
       localtime_r(&time,&t);
       char buf[64];
       strftime(buf,sizeof(buf),m_format.c_str(),&t);
       os << buf;
      }
    private:
      std::string m_format;
  };

  class FileFormatItem: public LogFormatter::FormatItem{
    public:
      FileFormatItem(const std::string& s =""){}
      void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event ) override{
          os << event->getFile();
      }
  };

  class LineFormatItem: public LogFormatter::FormatItem{
    public:
      LineFormatItem(const std::string& s =""){}
      void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event ) override{
          os << event->getLine();
      }
  };

  class NewLineFormatItem: public LogFormatter::FormatItem{
    public:
      NewLineFormatItem(const std::string& s =""){}
      void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event ) override{
          os << std::endl;
      }
  };

  class StringFormatItem: public LogFormatter::FormatItem{
    public:
      StringFormatItem(const std::string& s =""):str(s){}
      void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event ) override{
          os << str;
      }
    private:
      std::string str;
  };

  void LogFormatter::init(){
    //默认的pattern:%d{%Y-%m-%d %H:%M:%S}%T%t%T%F[%p]%T[%c]%T%f:%l%m%n"
    /* %m 消息体
     * %p level
     * %c 日志名称
     * %r 启动后的时间
     * %t 线程ID 
     * %n 回车换行
     * %f 文件名
     * %T 制表符
     * %F 协程ID
     * %l 行号
     * */
    //对其解析
    std::string s;
    std::vector<std::tuple<std::string,std::string,int>> vec;
    for(size_t i = 0; i<m_pattern.size();++i){
      
      if(m_pattern[i] != '%'){
          s.append(1,m_pattern[i]);
          continue;
      }

      //针对%% 
      if(i+1 < m_pattern.size() && m_pattern[i+1] == '%'){
        s.append(1,'%');
        continue;
      }

      //解析 %x
      size_t j = i+1;
      std::string str;
      std::string fmt;
      size_t fmt_begin;
      int status = 0;

      while(j < m_pattern.size()){
        
        //%x --> 若x不是字母，不是{ 或 }，则会break
        if(!status && (!isalpha(m_pattern[j]) && m_pattern[j] != '{' && m_pattern[j] != '}')){
          str = m_pattern.substr(i+1,j-i-1);
          break;
        }
          
      
        if(status == 0){   
          if(m_pattern[j] == '{'){  //%x{yyyy}---> str记录x,fmt记录yyyy
            //以i+1为起点，向后截出j-i-1个字符
            str  = m_pattern.substr(i+1,j-i-1);
            status = 1;
            fmt_begin = j+1;
            ++j;
            continue;
          }
        }else if(status == 1){
          if(m_pattern[j] == '}'){
            fmt = m_pattern.substr(fmt_begin,j-fmt_begin);
            status = 0;
            ++j;
            break;
          }
        }
        //other cases:字母
        ++j;
        if(j == m_pattern.size()){
          if(str.empty())
            str = m_pattern.substr(i+1);
        }
      }
      
      if(status == 0){    // %xxx 格式 或则 %x{yyyy}
        if(!s.empty()){
          vec.push_back(std::make_tuple(s,std::string(),0));  //Ex:0表示为string,1表示为pattern
          s.clear();
        }
        vec.push_back(std::make_tuple(str,fmt,1));
        i = j-1;
      }else if(status == 1){ // %xxx{  格式 --> 解析错误
        std::cout << "pattern parse error: " << m_pattern << "-" << m_pattern.substr(i) << std::endl;
        //记录error 
        m_error = true;
        vec.push_back(std::make_tuple("<<pattern_error>>",fmt,0));
      }
    }

    if(!s.empty())
      vec.push_back(std::make_tuple(s,std::string(),0));


//-----------------------------------------Global And Static -----------------------------------------------
  //建立映射关系
  //function:接受一个string,返回对应的formatItem
    static std::map<std::string,std::function<LogFormatter::FormatItem::ptr(const std::string& str)>> s_format_items = {
#define XX(str,fmtItem)\
      {#str,[](const std::string& fmt){return LogFormatter::FormatItem::ptr(new fmtItem(fmt));}}
      XX(m,MessageFormatItem),
      XX(p,LevelFormatItem),
      XX(r,ElapseFormatItem),
      XX(n,NewLineFormatItem),
      XX(d,DateFormatItem),
      XX(t,ThreadIdFormatItem),
      XX(f,FileFormatItem),
      XX(c,NameFormatItem),
      XX(l,LineFormatItem),
      XX(T,TableFormatItem),
      XX(F,FiberIdFormatItem),
      XX(N,ThreadNameFormatItem),
#undef XX
    };
  
    for(auto& i:vec){
      if(std::get<2>(i) == 0)
        m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
      else {
        auto it = s_format_items.find(std::get<0>(i));  //找到pattern，调用function，得到item
        if(it == s_format_items.end()){
           m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %"+std::get<0>(i)+">>")));
           m_error = true;

        }
        else 
           m_items.push_back(it->second(std::get<1>(i)));
      }
    }
  }
    //Data Bean
    struct LogAppenderDefine {
      int type = 0; //1 File, 2 Stdout
      LogLevel::Level level = LogLevel::UNKNOWN;
      std::string formatter;
      std::string file;

      bool operator==(const LogAppenderDefine& oth) const {
          return type == oth.type
              && level == oth.level
              && formatter == oth.formatter
              && file == oth.file;
      }
    };

    struct LogDefine {
      std::string name;
      LogLevel::Level level = LogLevel::UNKNOWN;
      std::string formatter;
      std::vector<LogAppenderDefine> appenders;

      bool operator==(const LogDefine& oth) const {
          return name == oth.name
              && level == oth.level
              && formatter == oth.formatter
              && appenders == appenders;
      }

      bool operator<(const LogDefine& oth) const {
          return name < oth.name;
      }

      bool isValid() const {
          return !name.empty();
      }
   };

template<>
class LexicalCast<std::string, LogDefine> {
public:
    LogDefine operator()(const std::string& v) {
        YAML::Node n = YAML::Load(v);
        LogDefine ld;
        if(!n["name"].IsDefined()) {
            std::cout << "log config error: name is null, " << n
                      << std::endl;
            throw std::logic_error("log config name is null");
        }
        ld.name = n["name"].as<std::string>();
        ld.level = LogLevel::FromString(n["level"].IsDefined() ? n["level"].as<std::string>() : "");
        if(n["formatter"].IsDefined()) {
            ld.formatter = n["formatter"].as<std::string>();
        }

        if(n["appenders"].IsDefined()) {
            //std::cout << "==" << ld.name << " = " << n["appenders"].size() << std::endl;
            for(size_t x = 0; x < n["appenders"].size(); ++x) {
                auto a = n["appenders"][x];
                if(!a["type"].IsDefined()) {
                    std::cout << "log config error: appender type is null, " << a
                              << std::endl;
                    continue;
                }
                std::string type = a["type"].as<std::string>();
                LogAppenderDefine lad;
                if(type == "FileLogAppender") {
                    lad.type = 1;
                    if(!a["file"].IsDefined()) {
                        std::cout << "log config error: fileappender file is null, " << a
                              << std::endl;
                        continue;
                    }
                    lad.file = a["file"].as<std::string>();
                    if(a["formatter"].IsDefined()) {
                        lad.formatter = a["formatter"].as<std::string>();
                    }
                } else if(type == "StdoutLogAppender") {
                    lad.type = 2;
                    if(a["formatter"].IsDefined()) {
                        lad.formatter = a["formatter"].as<std::string>();
                    }
                } else {
                    std::cout << "log config error: appender type is invalid, " << a
                              << std::endl;
                    continue;
                }

                ld.appenders.push_back(lad);
            }
        }
        return ld;
    }
};
     //先将set中内容逐个提取到node中，通过strignstreamf << YAML::Node转为string
template<>
class LexicalCast<LogDefine, std::string> {
public:
    std::string operator()(const LogDefine& i) {
        YAML::Node n;
        n["name"] = i.name;
        if(i.level != LogLevel::UNKNOWN) {
            n["level"] = LogLevel::ToString(i.level);
        }
        if(!i.formatter.empty()) {
            n["formatter"] = i.formatter;
        }

        for(auto& a : i.appenders) {
            YAML::Node na;
            if(a.type == 1) {
                na["type"] = "FileLogAppender";
                na["file"] = a.file;
            } else if(a.type == 2) {
                na["type"] = "StdoutLogAppender";
            }
            if(a.level != LogLevel::UNKNOWN) {
                na["level"] = LogLevel::ToString(a.level);
            }

            if(!a.formatter.empty()) {
                na["formatter"] = a.formatter;
            }

            n["appenders"].push_back(na);
        }
        std::stringstream ss;
        ss << n;
        return ss.str();
    }
};

    sylar::ConfigVar<std::set<LogDefine> >::ptr g_log_define =   //Log的配置项 -----> 一系列Loggers
          sylar::Config::LookUp("logs",std::set<LogDefine>(), "logs configure");
    
    struct LogIniter{
      LogIniter(){
        g_log_define->addListener([](const std::set<LogDefine>& old_value,  
                                     const std::set<LogDefine>& new_value){
           SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "on_logger_conf_changed";
           for(auto& i: new_value){
              auto it = old_value.find(i);
              sylar::Logger::ptr logger;

              if(it == old_value.end())  //新增logger 
                  logger = SYLAR_LOG_LOGGER(i.name);
              else{
                if(!(i == *it))
                  logger = SYLAR_LOG_LOGGER(i.name);
              } 

              //公共操作
              logger->setLevel(i.level);
              
              if(!i.formatter.empty()){
                logger->setFormatter(i.formatter);
              }        

              logger->clearAppenders();
              for(auto& a: i.appenders){
                sylar::LogAppender::ptr ap;
                if(a.type == 1)
                   ap.reset(new FileLogAppender(a.file));
                else if(a.type == 2)
                   ap.reset(new StdoutLogAppender);
                ap->setLevel(a.level);
                if(!a.formatter.empty()) {
                      LogFormatter::ptr fmt(new LogFormatter(a.formatter));
                      if(!fmt->hasError()) {
                          ap->setFormatter(fmt);
                      } else {
                          std::cout << "log.name=" << i.name << " appender type=" << a.type
                                    << " formatter=" << a.formatter << " is invalid" << std::endl;
                      }
                  }
               logger->addAppender(ap);
              }
         }

           //old中有而new中没有 ---> 删除 ---> 并不真正删除(可能影响到某些全局变量) ----> 只要改logger不再写日志 ----> 调整其loglevel,删除appenders
           for(auto& i: old_value){
              auto it = new_value.find(i);
              if(it == new_value.end()){
                auto logger = SYLAR_LOG_LOGGER(i.name);
                logger->setLevel((LogLevel::Level)100);
                logger->clearAppenders();
              }
           }
        });
      }
    };
    //static对象会在main函数之前构造 ----> 调用构造函数 ----> 可在构造函数do sth
    static LogIniter __log_init;
    void LoggerManager::init(){
      return;     
    }



//-------------------------------------------------LoggerManager---------------------------------------------------
    LoggerManager::LoggerManager(){
      m_root.reset(new Logger);
      m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
      //root初始化好以后将其放入m_loggers中
      m_loggers[m_root->m_name] = m_root;
      init();
    }
    std::string LoggerManager::toYamlString(){
      MutexType::LockGuard lock_guard(m_mutex);
      YAML::Node node;
      for(auto& i:m_loggers)
        node.push_back(YAML::Load(i.second->toYamlString()));
      std::stringstream ss;
      ss<<node;
      return ss.str();
    }
    //不存在就创建语义
    Logger::ptr LoggerManager::getLogger(const std::string& name){
       MutexType::LockGuard lock_guard(m_mutex);
        auto it = m_loggers.find(name);
        if(it != m_loggers.end())
          return it->second;
        Logger::ptr logger(new Logger(name));
        logger->m_root = m_root;
        m_loggers[name] = logger;
        return logger;
    }
}
