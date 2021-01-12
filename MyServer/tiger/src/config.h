#ifndef MY_CONFIG_H
#define MY_CONFIG_H 


#include <string>
#include <memory>
#include <map>
#include <yaml-cpp/yaml.h>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <functional>
#include "log.h"

namespace tiger{


  class ConfigItemBase{
    public:
      typedef std::shared_ptr<ConfigItemBase> ptr;
      ConfigItemBase(const std::string& name,const std::string& descrption):m_name(name),m_description(descrption){}
      ~ConfigItemBase(){}

      //虚函数
      virtual std::string to_string() = 0;
      virtual void from_string(const std::string& s) = 0;
      virtual std::string get_type_name() const = 0;
      virtual std::string dump()  = 0;
      //通用get函数
      const std::string& getName()const{return m_name;}
      const std::string& getDescription()const{return m_description;}

    protected:
      std::string m_name;
      std::string m_description;
  };


//类型转换functor
    //从T1---->T2
    template<class T1,class T2>
      class LexicalCast{
       public:
        T2 operator()(const T1& val){
          return boost::lexical_cast<T2>(val);
        }
      };

    //偏特化版本
    
    //1)string-->vector<T2>
    template<class T2>
      class LexicalCast<std::string,std::vector<T2>>{
        public:
          std::vector<T2> operator()(const std::string& val){
            //先将string还原为YAML Node 
            YAML::Node node = YAML::Load(val);
            std::stringstream ss;
            std::vector<T2> res;
            //遍历子node
            for(size_t i=0; i<node.size(); i++){
              ss.str("");
              ss << node[i];
              res.push_back(LexicalCast<std::string,T2>()(ss.str()));   //若node[i]内嵌有复杂node,比如map node ---> 则递归调用其他片特化版本
            }
            return res;
          }
      };
    //1.1)vector<T1>-->string 
    template<class T1>
      class LexicalCast<std::vector<T1>,std::string>{
        public:
          std::string operator()(const std::vector<T1>& v){
            //考虑到T1也可能为复杂类型 ---> 递归调用其他片特化版本
           YAML::Node node;
           for(auto& i: v)
             node.push_back(YAML::Load(LexicalCast<T1,std::string>()(i)));
           std::stringstream ss;
           ss << node;
           return ss.str();
          }
      };
  //其他stl容器的支持
  //list
  template<class T2>
    class LexicalCast<std::string,std::list<T2> >{
        public:
          std::list<T2> operator() (const std::string& str ){
            YAML::Node node = YAML::Load(str);
            std::list<T2> vec;
            std::stringstream ss;
            for(size_t i = 0; i<node.size(); i++){
              ss.str("");
              ss << node[i];
              vec.push_back(LexicalCast<std::string,T2>()(ss.str()));
            }
            return vec;
          }
    };
  template<class T1>
    class LexicalCast<std::list<T1>,std::string >{
        public:
          std::string operator() (const std::list<T1>& vec ){
              YAML::Node node;
              for(auto& i:vec)
                node.push_back(YAML::Load(LexicalCast<T1,std::string>()(i))); 
              std::stringstream ss;
              ss<<node;
              return ss.str();
          }
    };

   //set
  template<class T2>
    class LexicalCast<std::string,std::set<T2> >{
        public:
          std::set<T2> operator() (const std::string& str ){
            YAML::Node node = YAML::Load(str);
            std::set<T2> vec;
            std::stringstream ss;
            for(size_t i = 0; i<node.size(); i++){
              ss.str("");
              ss << node[i];
              vec.insert(LexicalCast<std::string,T2>()(ss.str()));
            }
            return vec;
          }
    };
  template<class T1>
    class LexicalCast<std::set<T1>,std::string >{
        public:
          std::string operator() (const std::set<T1>& vec ){
              YAML::Node node;
              for(auto& i:vec)
                node.push_back(YAML::Load(LexicalCast<T1,std::string>()(i))); 
              std::stringstream ss;
              ss<<node;
              return ss.str();
          }
    };

  //unordered_set
  template<class T2>
    class LexicalCast<std::string,std::unordered_set<T2> >{
        public:
          std::unordered_set<T2> operator() (const std::string& str ){
            YAML::Node node = YAML::Load(str);
            std::unordered_set<T2> vec;
            std::stringstream ss;
            for(size_t i = 0; i<node.size(); i++){
              ss.str("");
              ss << node[i];
              vec.insert(LexicalCast<std::string,T2>()(ss.str()));
            }
            return vec;
          }
    };
  template<class T1>
    class LexicalCast<std::unordered_set<T1>,std::string >{
        public:
          std::string operator() (const std::unordered_set<T1>& vec ){
              YAML::Node node;
              for(auto& i:vec)
                node.push_back(YAML::Load(LexicalCast<T1,std::string>()(i))); 
              std::stringstream ss;
              ss<<node;
              return ss.str();
          }
    };

  //map
  template<class T2>
    class LexicalCast<std::string,std::map<std::string, T2> >{
        public:
          std::map<std::string,T2> operator() (const std::string& str ){
            YAML::Node node = YAML::Load(str);
            std::map<std::string, T2> vec;
            std::stringstream ss;
            for(auto it=node.begin(); it != node.end(); it++){
              ss.str("");
              ss << it->second;
              vec.insert(std::make_pair(it->first.Scalar(),LexicalCast<std::string,T2>()(ss.str())));
            }
            return vec;
          }
    };
  template<class T1>
    class LexicalCast<std::map<std::string,T1>,std::string >{
        public:
          std::string operator() (const std::map<std::string,T1>& vec ){
              YAML::Node node;
              for(auto& i:vec){
                node[i.first] = YAML::Load(LexicalCast<T1, std::string>()(i.second));
              }
              std::stringstream ss;
              ss<<node;
              return ss.str();
          }
    };
  template<class T2>
    class LexicalCast<std::string,std::unordered_map<std::string, T2> >{
        public:
          std::unordered_map<std::string,T2> operator() (const std::string& str ){
            YAML::Node node = YAML::Load(str);
            std::unordered_map<std::string, T2> vec;
            std::stringstream ss;
            for(auto it=node.begin(); it != node.end(); it++){
              ss.str("");
              ss << it->second;
              vec.insert(std::make_pair(it->first.Scalar(),LexicalCast<std::string,T2>()(ss.str())));
            }
            return vec;
          }
    };
  template<class T1>
    class LexicalCast<std::unordered_map<std::string,T1>,std::string >{
        public:
          std::string operator() (const std::unordered_map<std::string,T1>& vec ){
              YAML::Node node;
              for(auto& i:vec){
                node(i.first)  = YAML::Load(LexicalCast<T1, std::string>()(i.second));
              }
              std::stringstream ss;
              ss<<node;
              return ss.str();
          }
    };



  template<class T,class fromString = LexicalCast<std::string,T>, class toString= LexicalCast<T,std::string> >
  class ConfigItem : public ConfigItemBase{
    public:
      typedef std::shared_ptr<ConfigItem> ptr;
      ConfigItem(const std::string& name,const T& value,const std::string& descrption):ConfigItemBase(name,descrption),m_value(value){}

      //参数:旧值和新值  ----> 可能需要用clear旧值，又或者需要用旧值构建新值
      typedef  std::function<void(T& old_value,const T& new_value)> on_change_call_back;

      void set_value(const T& value){
        if(m_value == value)  //新旧值一样
          return;
        //调用回调函数
        for(auto& it:m_cbs)
          it.second(m_value,value);
        m_value = value;
      }
      T& get_value() const {return m_value;}
      
      std::string to_string() override{
        try{
          return toString()(m_value);
        }catch(std::exception& e){
          TIGER_LOG_ERROR(TIGER_LOG_LOGGER("system")) << "ConfigItem toString() exception:" << e.what()
                                                      <<"type:" << typeid(T).name() <<"convert to string failed";
        }
      return "";
    }
    void from_string(const std::string& s) override{
      try{
        set_value(fromString()(s));
      }catch(std::exception& e){
        TIGER_LOG_ERROR(TIGER_LOG_LOGGER("system")) << "ConfigItem from_string() exception:" << e.what()
                                                    <<"type: " << typeid(T).name() <<" convert from string failed";
      }
    }
    std::string get_type_name() const override{
      return typeid(T).name();
    }
    std::string dump() {
      std::stringstream ss;
      ss << "m_name:" << m_name << "--"
          << "m_value:" << to_string() << "--"
          << "m_description:" << m_description;
      return ss.str();
    }

    //回调相关
    uint64_t add_listener(on_change_call_back cb){
      static uint32_t key = 0;        //使用静态变量，递增作为key
      m_cbs[key++] = cb;
      return key-1;        //将key返回用户
    }
    void delete_listener(uint64_t key){
      m_cbs.erase(key);
    }
    void clear_listener(){
      m_cbs.clear();
    }

  private:
    T m_value;
    //存储回调函数，支持配置变更通知
    std::map<uint64_t,on_change_call_back> m_cbs;
};



class ConfigManager{
  public:
    typedef  std::map<std::string,ConfigItemBase::ptr> map_type;
    //加载yaml文件，将node转为string，用string构建ConfigItem
    static void load_from_yaml(const std::string& file);

    template<class T>
    static typename ConfigItem<T>::ptr LookUp(const std::string& name);
    //找不到就创建语义
    template<class T>
    static typename ConfigItem<T>::ptr LookUp(const std::string& name,const T& arg,
                                      const std::string& description = "");
      
    static ConfigItemBase::ptr LookUpBase(const std::string& name);
    
  private:
    //辅助函数
    //记:static函数只与类相关,没有this指针，无法调用成员函数
    //记:static函数不要使用默认参数
    static void get_all_nodes(const std::string& prefix,const YAML::Node node,
                              std::vector<std::pair<std::string,YAML::Node>>& output);

  public:
   static map_type& GetConfigItems(){
      static map_type config_items;
      return config_items;
   }
};

     template<class T>
       typename ConfigItem<T>::ptr ConfigManager::LookUp(const std::string& name){
         auto it = GetConfigItems().find(name);
         if(it == GetConfigItems().end())
           return  nullptr;
         return std::dynamic_pointer_cast<ConfigItem<T>>(it->second);
     }

    template<class T>
    typename ConfigItem<T>::ptr ConfigManager::LookUp(const std::string& name,const T& arg,
        const std::string& description){
        auto i = GetConfigItems().find(name);
        if(i != GetConfigItems().end())
         return std::dynamic_pointer_cast<ConfigItem<T>>(i->second);

        if(name.find_last_not_of("abcdeghijklmnopqrstuvwxyz"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "123456789_.") != std::string::npos){
          TIGER_LOG_ERROR(TIGER_LOG_ROOT) << "LookUp name invalid " << name;
          throw std::invalid_argument(name);
        }
        typename  ConfigItem<T>::ptr  new_item(new ConfigItem<T>(name,arg,description));
        GetConfigItems()[name] = new_item;
        return new_item;
      }

}

#endif
