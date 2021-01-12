#include <functional>
#include <tuple>
#include <map>
#include <iostream>
#include <time.h>
#include <algorithm>
#include "config.h"
#include "log.h"







namespace tiger{



///-----------------------Logger-----------------
  const std::string LogLevel::LevelToStr(LogLevel::Level level){
    //采用macro实现
    switch(level){
#define XX(name)\
      case LogLevel::name: \
       return #name;
      XX(DEBUG)
      XX(INFO)
      XX(ERROR)
      XX(WARN)
      XX(FATAL)
#undef XX
      default:
      return "UNKNOWN";
    }
  }


///-----------------------LogFormatter--------------

  LogFormatter::LogFormatter(const std::string& pattern):m_pattern(pattern) {parse_pattern();}

  class FileItem : public LogFormatter::FormatItem{
    public:
      FileItem(const std::string& str=""){}
       void format(std::stringstream& os,LogEvent::ptr event) override{
            os << event->get_file();
      }
  };
  class LineItem: public LogFormatter::FormatItem{
    public:
       LineItem(const std::string& str=""){}
       void format(std::stringstream& os,LogEvent::ptr event) override{
            os << event->get_line();
      }
  };

  class LoggerNameItem : public LogFormatter::FormatItem{
    public:
       LoggerNameItem(const std::string& str=""){}
       void format(std::stringstream& os,LogEvent::ptr event) override{
            os << event->get_logger_name();
      }
  };

  class ThreadNameItem : public LogFormatter::FormatItem{
    public:
       ThreadNameItem(const std::string& str=""){}
       void format(std::stringstream& os,LogEvent::ptr event) override{
            os << event->get_thread_name();
      }
  };
  class TimeItem : public LogFormatter::FormatItem{
    public:
       TimeItem(const std::string& str = "%Y-%m-%d %H:%M:%S"):m_time_format(str){}
       void format(std::stringstream& os,LogEvent::ptr event) override{
         time_t time = event->get_time();
         struct tm m_tm;
         localtime_r(&time,&m_tm);
         char buf[32];
         strftime(buf,sizeof(buf),m_time_format.c_str(),&m_tm);
         os << buf;

      }
     private:
       std::string m_time_format;

  };
  class ThreadIdItem : public LogFormatter::FormatItem{
    public:
       ThreadIdItem(const std::string& str=""){}
       void format(std::stringstream& os,LogEvent::ptr event) override{
            os << event->get_thread_id();
      }
  };

  class FiberIdItem : public LogFormatter::FormatItem{
    public:
       FiberIdItem(const std::string& str=""){}
       void format(std::stringstream& os,LogEvent::ptr event) override{
            os << event->get_fiber_id();
      }
  };

  class ContentItem : public LogFormatter::FormatItem{
    public:
       ContentItem(const std::string& str=""){}
       void format(std::stringstream& os,LogEvent::ptr event) override{
            os << event->get_content();
      }
  };

  class LevelItem : public LogFormatter::FormatItem{
    public:
       LevelItem(const std::string& str=""){}
       void format(std::stringstream& os,LogEvent::ptr event) override{
            os << event->get_level();
      }
  };

  class NewLineItem : public LogFormatter::FormatItem{
    public:
       NewLineItem(const std::string& str=""){}
       void format(std::stringstream& os,LogEvent::ptr event) override{
            os << "\n";
      }
  };

  class TableItem : public LogFormatter::FormatItem{
    public:
       TableItem(const std::string& str=""){}
       void format(std::stringstream& os,LogEvent::ptr event) override{
            os << "\t";
      }
  };
  class StringItem : public LogFormatter::FormatItem{
    public:
       StringItem(const std::string& str=""):m_str(str){}
       void format(std::stringstream& os,LogEvent::ptr event) override{
            os << m_str;
      }
    private:
      std::string m_str;
  };

 void LogFormatter::parse_pattern(){

    //解析后的数据:
    //0 ---> 普通字符串
    //1 ---> pattern字符 或者 pattern字符+额外的pattern
  std::vector<std::tuple<int,std::string,std::string>> v;
  int s = 0;
  std::string normal_str;
  std::string fmt,extra_fmt;
  //采用状态机进行解析
  for(size_t i=0;i<m_pattern.size();){
    switch(s){
      case 0:
        if(m_pattern[i] == '%')
          s = 1;
        else 
          s= 3;
        break;
      case 1:
        ++i;  //现在i指向%的下一个
        if(i<m_pattern.size())
          fmt.push_back(m_pattern[i]);
        ++i;
        if(m_pattern[i] == '%'){
          v.push_back(make_tuple(1,fmt,""));
          fmt.clear();
          s = 0;
        }else if(m_pattern[i] == '{')
          s = 2;
        else{
          v.push_back(make_tuple(1,fmt,""));
          fmt.clear();
          s = 0;
        }
        break;
      case 2:
        ++i;  //现在i指向{的下一个
        while(i<m_pattern.size() && m_pattern[i] != '}')
          extra_fmt.push_back(m_pattern[i++]);
        if(m_pattern[i] != '}'){
          m_error = true;
          m_items.push_back(FormatItem::ptr(new StringItem("<error format pattern:lack }>")));
        }
        else
          v.push_back(make_tuple(1,fmt,extra_fmt));
        fmt.clear();
        extra_fmt.clear();
        s = 0;
        ++i;  //现在i指向}的下一个
        break;
      case 3:
        while(i<m_pattern.size() && m_pattern[i] != '%')
          normal_str.push_back(m_pattern[i++]);
        v.push_back(make_tuple(0,normal_str,""));
        normal_str.clear();
        s = 0;
        break;
    }
  }

  

   //pattern字符与FormatItem的映射
  static std::map<std::string,std::function<LogFormatter::FormatItem::ptr(const std::string& item_fmt)> > m_map={

#define XX(str,Item)\
   {#str,[](const std::string& item_fmt){return LogFormatter::FormatItem::ptr(new Item(item_fmt));}}
    XX(m,ContentItem),         //内容
    XX(p,LevelItem),
    XX(d,TimeItem),
    XX(c,LoggerNameItem),
    XX(t,ThreadIdItem),
    XX(n,NewLineItem),
    XX(l,LineItem),
    XX(T,TableItem),
    XX(F,FiberIdItem),
    XX(N,ThreadNameItem),
    XX(f,FileItem),
#undef XX

  };

  //将解析的pattern字符映射成FormatItems,并存储
  for(const auto& i:v){
    if(std::get<0>(i) == 0) //normal string
      m_items.push_back(FormatItem::ptr(new StringItem(std::get<1>(i))));
    else{
      auto it = m_map.find(std::get<1>(i));
      if(it == m_map.end()){ //非法字符，用StringItem表达错误
        m_error = true;
        m_items.push_back(FormatItem::ptr(new StringItem("<error format pattern:"+std::get<1>(i)+">")));
        continue;
      }
     if(std::get<0>(i) == 1)  //pattern字符
      m_items.push_back(it->second(std::get<2>(i)));
    }
  }

}

  std::string LogFormatter::format(LogEvent::ptr event){
    std::stringstream os;
    for(auto& it:m_items)
      it->format(os,event);
    return os.str();
  }

 LogAppender::LogAppender(LogFormatter::ptr p){
   if(p == nullptr)
     p.reset(new(LogFormatter));  //确保Appender含有默认的Formatter
    m_formatter = p;
}
 void StdoutLogAppender::log(LogEvent::ptr event){
      std::cout << m_formatter->format(event);
 }
 std::string StdoutLogAppender::to_yaml_string(){
      YAML::Node ap_node;
      ap_node["type"] = "StdoutLogAppender";
      if(!m_formatter->get_pattern().empty())
       ap_node["formatter"] = m_formatter->get_pattern();
      std::stringstream ss;
      ss<< ap_node;
      return ss.str();
 }


  FileLogAppender::FileLogAppender(const std::string& file,LogFormatter::ptr p):LogAppender(p),m_filename(file){
          open_file();
  }
 void FileLogAppender::log(LogEvent::ptr event){
        auto now = event->get_time();
        if(now >= (m_lastTime+3)){
          open_file();
          m_lastTime = now;
        }
        m_file_stream << m_formatter->format(event);
 }
 
 void FileLogAppender::open_file(){
   if(m_file_stream)
     m_file_stream.close();
   m_file_stream.open(m_filename);
 }

 std::string FileLogAppender::to_yaml_string(){
      YAML::Node ap_node;
      ap_node["type"] = "FileLogAppender";
      ap_node["file"] = m_filename;
      if(!m_formatter->get_pattern().empty())
       ap_node["formatter"] = m_formatter->get_pattern();
      std::stringstream ss;
      ss<< ap_node;
      return ss.str();
 }

///-------------------------Logger------------

 void Logger::log(LogEvent::ptr event,LogLevel::Level level){
    if(event == nullptr)
      return;
    event->set_logger_level(level);
    event->set_logger_name(m_name);
    auto& appenders = m_appenders;
    if(m_appenders.empty()){
      //使用root_loger的appender进行输出
      auto root_logger = LoggerMgr::GetInstance()->get_root_logger();
      appenders = root_logger->get_appenders();
    }
    for(auto& i:appenders)
      i->log(event);
 }
    void Logger::debug(LogEvent::ptr event){
      log(event,LogLevel::DEBUG);
    }
    void Logger::error(LogEvent::ptr event){
      log(event,LogLevel::ERROR);
    }
    void Logger::warn(LogEvent::ptr event){
      log(event,LogLevel::WARN);
    }
    void Logger::fatal(LogEvent::ptr event){
      log(event,LogLevel::FATAL);
    }

    void Logger::set_formatter(const std::string& pattern) {
      m_formatter.reset(new LogFormatter(pattern));

    }
    void Logger::set_formatter(LogFormatter::ptr f){
     
     if(!f)
       return;
     m_formatter = f;
    }
   void Logger::sync_fomatter(){
      for(auto& i:m_appenders)
        i->set_formatter(m_formatter);
   }
   void Logger::del_appender(LogAppender::ptr apendr){
      auto it = find(m_appenders.begin(),m_appenders.end(),apendr);
      if(it != m_appenders.end())
        m_appenders.erase(it);
   }

  void Logger::clear_appenders(){
        m_appenders.clear();
  } 

  std::string Logger::to_yaml_string(){
    //先将其转为YAML Node 
    YAML::Node log_node;
    log_node["name"] = m_name;
    if(!get_formatter()->get_pattern().empty())
       log_node["formatter"] = get_formatter()->get_pattern();
    for(auto& i:m_appenders){
      YAML::Node ap_node = YAML::Load(i->to_yaml_string());
      log_node["appenders"].push_back(ap_node);
    }
    std::stringstream ss;
    ss << log_node;
    ss << "\n";
    return ss.str();
  }





  LoggerManager::LoggerManager(){
    m_root_logger.reset(new Logger("root"));
    m_root_logger->add_appender(LogAppender::ptr(new StdoutLogAppender));
    m_loggers["root"] = m_root_logger;
  }
  Logger::ptr LoggerManager::LookUp(const std::string& name){

    auto it = m_loggers.find(name);
    if(it == m_loggers.end())
        m_loggers[name].reset(new Logger(name));
    return m_loggers[name];
  }
  void LoggerManager::del_logger(const std::string& name){
      m_loggers.erase(name);
  }
   void LoggerManager::add_logger(const std::string& name,Logger::ptr p){
      m_loggers.insert(std::make_pair(name,p));
  }
   void LoggerManager::add_logger(const std::string& name){
      m_loggers.insert(std::make_pair(name,Logger::ptr(new Logger(name))));
  }
  std::string LoggerManager::to_yaml_string(){
    std::stringstream ss;
    for(auto& i : m_loggers)
      ss << i.second->to_yaml_string();
    return ss.str();
  }

  



//-----------------------------与配置系统的整合------------------------------------------------------------

struct LogAppenderDefine{
  
  int m_type = 0;     //1--FileLogAppender  2--StdoutLogAppender 
  std::string m_file = "";  //针对FileLogAppender 
  std::string m_formatter = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n";
  //定义 == 
  bool operator==(const LogAppenderDefine& rhs) const{
    return m_type == rhs.m_type && 
           m_file == rhs.m_file &&
           m_formatter == rhs.m_formatter;
  }
};

struct LoggerDefine{
  std::string m_name;
  std::vector<LogAppenderDefine> m_appenders;
  std::string m_formatter = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n";

  

  //最后的存储形式: logs set<LoggerDefine> descrption    ---->将多个logger存放在set中，只占一个ConfigItem
  //set便于快速查找，所以需要定义相关的比较运算符
  
  bool operator<(const LoggerDefine& rhs) const{
    return m_name < rhs.m_name;
  }
  bool operator==(const LoggerDefine& rhs) const{
    return m_name  == rhs.m_name &&
           m_appenders == rhs.m_appenders &&
           m_formatter == rhs.m_formatter;
  }
  bool operator!=(const LoggerDefine& rhs) const{
      return !(*this == rhs);
  }
};

//LoggerDefine  <---------------------> string 
template<>
class LexicalCast<std::string,LoggerDefine>{
  public:
    LoggerDefine operator()(const std::string& str){
      //先将其还原YAML::Node，再逐个标签的解析
      YAML::Node node = YAML::Load(str);
      LoggerDefine res;
      //name 标签
      if(!node["name"].IsDefined()){
        std::cout << "log cofig error: name is null" << node << std::endl;
        throw std::logic_error("log config name is null");
      }
      res.m_name = node["name"].as<std::string>();
      
      //formatter为可选项
      if(node["formatter"].IsDefined())
        res.m_formatter = node["formatter"].as<std::string>();

      //appender为数组，需要逐个解析
      if(node["appenders"].IsDefined()){
        for(size_t i=0;i<node["appenders"].size();i++){
          auto appender = node["appenders"][i];
          LogAppenderDefine tmp;
          if(!appender["type"].IsDefined()){
            //////
          }
          std::string type = appender["type"].as<std::string>();
          if(type == "FileLogAppender")
            tmp.m_type = 1;
          else if(type == "StdoutLogAppender")
            tmp.m_type = 2;
          else{
            std::cout << "log config error: appender type is null, " <<  appender << std::endl;
            continue;
          }
          if(tmp.m_type == 1){ //file appender -->解析file
            if(!appender["file"].IsDefined()){
               std::cout << "log config error: appender file is null, " <<  appender << std::endl;
              continue;
            }
            tmp.m_file = appender["file"].as<std::string>();
          }
          
          //formatter为可选项
          if(appender["formatter"].IsDefined())
            tmp.m_formatter = appender["formatter"].as<std::string>();
          res.m_appenders.push_back(tmp);
        }
      }
      return res;
    }
};



template<>
class LexicalCast<LoggerDefine,std::string>{
  public:
    std::string operator()(const LoggerDefine& logger){
      //将logger中的每个元素抽取出来，装入YAML Node中，再将Node通过stringstram序列化(YAML Node格式的字符串)
      YAML::Node log_node;
      std::string res;
      log_node["name"] = logger.m_name;
      if(logger.m_formatter != "")
        log_node["formatter"] = logger.m_formatter;
      
      //处理appenders
      for(size_t i=0;i<logger.m_appenders.size();i++){
        YAML::Node appender_node;
        auto appender = logger.m_appenders[i];
        if(appender.m_type == 1){  //FileLogAppender
          appender_node["type"] = "FileLogAppender";
          appender_node["file"] = appender.m_file;
        }else if(appender.m_type == 2){
          appender_node["type"] = "StdoutLogAppender";
        }
        if(!appender.m_formatter.empty())
           appender_node["formatter"] = appender.m_formatter;
        log_node["appenders"].push_back(appender_node);
      }
      std::stringstream ss;
      ss << log_node;
      return ss.str();
    };
};



//在程序开始前，静态配置Log的配置项
static ConfigItem<std::set<LoggerDefine>>::ptr log_config = ConfigManager::LookUp("logs",std::set<LoggerDefine>(),"log configure items");

struct LogIniter{
  LogIniter(){
    //主要是为其添加回调函数
    //回调函数的作用:根据新值与旧值，判断操作为增删改的哪一个
    
    log_config->add_listener([](std::set<LoggerDefine>& old_value,const std::set<LoggerDefine>& new_value){
              
      std::cout << "---------- log Config changed-------------------" << std::endl;        

      //新增Logger:old_value没有,new_value中有
      auto old = old_value.begin();
      for(auto& i: new_value){
        
        if(old_value.find(i) == old_value.end())  //新增
            
        if( (old =old_value.find(i)) != old_value.end()){  //修改
            if(*old == i)            //一模一样
                return;
           //修改:二者里面都有 ---> 对于set,只能将原来的删除，添加新的
                std::cout << "come here-------" << std::endl;
           old_value.erase(i);
        }
        auto new_logger = TIGER_LOG_LOGGER(i.m_name);
        new_logger->clear_appenders();
        new_logger->set_formatter(i.m_formatter);
        for(auto& ap : i.m_appenders){
          if(ap.m_type == 1)
            new_logger->add_appender(FileLogAppender::ptr(
                                       new FileLogAppender(
                                         ap.m_file,
                                         LogFormatter::ptr(new LogFormatter(ap.m_formatter))
                                       )));
          else if(ap.m_type == 2)
             new_logger->add_appender(StdoutLogAppender::ptr(
                                       new StdoutLogAppender(
                                         LogFormatter::ptr(new LogFormatter(ap.m_formatter))
                                       )));
        }
       }

      //删除判断:old_value中有，new_value中没有
      for(auto& i : old_value)
        if(new_value.find(i) == new_value.end())
          LoggerMgr::GetInstance()->del_logger(i.m_name);

   });
 } 
};

static LogIniter log_initer ;




}
