#ifndef CONFIG_H
#define CONFIG_H 

#include <memory>
#include <sstream>
#include <string>
#include "../sylar/log.h"
#include <yaml-cpp/yaml.h>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include "thread.h"


//约定优于配置 

namespace sylar{

  class ConfigVarBase{
    public:
      typedef std::shared_ptr<ConfigVarBase> ptr;
      ConfigVarBase(const std::string& name,const std::string& description = "")
        :m_name(name),m_description(description){
          //将name转为小写
          std::transform(m_name.begin(),m_name.end(),m_name.begin(),::tolower);
        }
      virtual ~ConfigVarBase(){}


      const std::string& getName()const{return m_name;}
      void  setName(const std::string& name){m_name = name;}
      const std::string& getDescription()const{return m_description;}
      
      //转为string，方便
      virtual std::string toString() = 0;
      //解析string
      virtual bool fromString(const std::string& val) = 0;
      virtual  std::string getTypeName() const  = 0;
    private:
      std::string m_name;
      std::string m_description;
  };
  

  //将F转为T类型
  template<class T1,class T2>
  class LexicalCast{
    public:
     T2 operator()(const T1& val){
        return boost::lexical_cast<T2>(val);
     } 
  };

  //偏特化
  
 //string ---> vector<>
  template<class T2>
    class LexicalCast<std::string,std::vector<T2> >{
        public:
          std::vector<T2> operator() (const std::string& str ){
            //将string转为一个YAMLNode
            YAML::Node node = YAML::Load(str);
            std::vector<T2> vec;
            //进行遍历，拿出每个元素，构件vector
            std::stringstream ss;
            for(size_t i = 0; i<node.size(); i++){
              ss.str("");
              ss << node[i];
              //若为简单类型，则调用第一个版本，若为复杂类型，则调用其他版本 ---> 形成递归调用
              vec.push_back(LexicalCast<std::string,T2>()(ss.str()));
            }
            return vec;
          }
    };
  //vector<> ---> string 
  template<class T1>
    class LexicalCast<std::vector<T1>,std::string >{
        public:
          std::string operator() (const std::vector<T1>& vec ){
              YAML::Node node;
              //因为T1也可能为复杂类型，也需要调用每个LexicalCast进行转换
              for(auto& i:vec)
                node.push_back(YAML::Load(LexicalCast<T1,std::string>()(i))); //T1 ---> strings ---> YAMLnode ---> string
                
              std::stringstream ss;
              ss<<node;
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


  //对自定义类型的支持


  //一个配置项:name，description，value
  //配置文件的格式 <------> string
  template<class T,class FromStr = LexicalCast<std::string,T>, class ToStr = LexicalCast<T,std::string> >
    class ConfigVar : public ConfigVarBase{
      public:
        //读多写少，采用读写锁
        typedef RWMutex  MutexType;
        typedef std::shared_ptr<ConfigVar> ptr;

        //配置的通知机制:配置项发生改变时，通过回调函数进行相应的通知或处理。比如clear some data
        typedef std::function<void (const T& old_value,const T& new_value)> on_change_call_back;
        
        ConfigVar(const std::string& name, const T& val,const std::string& description ="")
                :ConfigVarBase(name,description),m_val(val){}
        
          const T getValue() const {
            MutexType::ReadLockGuard lock_guard(m_mutex);

            return m_val;
          }
          void setValue(const T& val){
            {
            MutexType::ReadLockGuard lock_guard(m_mutex);
            if(val == m_val)   //新值 == 旧值
              return;
            for(auto& i:m_cbs)   //值不一样 ----> 调用回调函数，做相应处理
              i.second(m_val,val);
            }
            MutexType::WriteLockGuard lock_guard(m_mutex);
            m_val = val;
          }

        std::string toString() override{
          try{
            //考察m_val是否能转为string; ---> 简单类型 return boost::lexical_cast<std::string>(m_val);
            MutexType::ReadLockGuard lock_guard(m_mutex);
            return ToStr()(m_val);
          }catch(std::exception& e){
            //typeid().name:获取类型名
              SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception" << e.what() 
                                                << "convert: " << getTypeName() << "to string";
          }
          return  "";
        }

        bool fromString(const std::string& val) override {
          try{
            //简单类型:m_val = boost::lexical_cast<T>(val);
            setValue(FromStr()(val));
          }catch(std::exception& e){
              SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::fromString exception" << e.what()
                                                << " convert string to: " << typeid(m_val).name();
          }
          return false;
        }

        std::string getTypeName() const override{return typeid(T).name();}

        uint64_t addListener(on_change_call_back cb){
          //用内部给每个回调函数赋予一个独一无二的key并将该key返回给user
            static uint64_t key = 0;
            MutexType::WriteLockGuard lock_guard(m_mutex);
            ++key;
            m_cbs[key] = cb;
            return key;
        }
        void delListener(uint64_t key){
            MutexType::WriteLockGuard lock_guard(m_mutex);
            m_cbs.erase(key);
        }
        on_change_call_back getListener(uint64_t key){
            MutexType::ReadLockGuard lock_guard(m_mutex);
            auto it = m_cbs.find(key);
            return it == m_cbs.end() ? nullptr : it->second;
        }
        void clearListener(){
            MutexType::WriteLockGuard lock_guard(m_mutex);
            m_cbs.clear();
        }
      private:
        T m_val;;
        //用map的原因:functional没有比较函数 ----> 如何找到删除呢? ---> 添加索引即可 ---> 索引要求唯一
        std::map<uint64_t,on_change_call_back> m_cbs;
        mutable MutexType m_mutex;
    };

  //管理类
  class Config{
    public:
      typedef RWMutex MutexType;
      // <名称,配置项> ----> 快速查找
      typedef std::map<std::string,ConfigVarBase::ptr> ConfigVarMap;
      //单纯地查找
      template<class T>
        static typename ConfigVar<T>::ptr LookUp(const std::string& name){
          auto it = GetDatas().find(name);
          return (it == GetDatas().end()) ? nullptr : std::dynamic_pointer_cast<ConfigVar<T>>(it->second); //需要父类指针转为子类指针
        }

      //查找，找不到就创建
      template<class T>
        static typename ConfigVar<T>::ptr LookUp(const std::string& name,const T& val,const std::string& description = ""){
          MutexType::WriteLockGuard  lock_guard(GetMutex());
          auto it = GetDatas().find(name);
          if(it != GetDatas().end()){   //容器中存在
              auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
              if(tmp){   //配置项存在
                SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "LookUp name=" << name << "exists";
                return tmp;
              }else{   //比如:同一key对应 int value和float value 
                       //int value已经存在，检索float value时，由于key相同 ----> 肯定能找到
                SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "LookUp name=" << name 
                                                 << "exists but type not" << typeid(T).name() 
                                                 << " real_type=" << it->second->getTypeName()
                                                 << " " << it->second->toString();
                return nullptr;
              }
          }
          //创建
          //先判断name的合法性
          if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._012345678") != std::string::npos){
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "LookUp name invalid " << name;
            throw std::invalid_argument(name);
          }
          typename ConfigVar<T>::ptr v(new ConfigVar<T>(name,val,description));
          GetDatas()[name] = v; 
          return v;
        }
      //load yaml树
      static void LoadFromYaml(const YAML::Node& root);
      static ConfigVarBase::ptr LookUpBase(const std::string& name);
      //用户自己给出访问配置项的方法
      static void Visit(std::function<void(ConfigVarBase::ptr)> cb);
    private:
      //静态变量初始化顺序不定
      //有可能其他静态变量可能先初始化并通过使用GetDatas()
      static ConfigVarMap& GetDatas(){
        MutexType::ReadLockGuard  lock_guard(GetMutex());
        static ConfigVarMap datas;
        return datas;
      }
      //保护全局静态成员datas
      //静态成员的初始化顺序不定
      static MutexType& GetMutex(){
         static MutexType m_mutex;
         return m_mutex;
      }
  };

}

#endif
