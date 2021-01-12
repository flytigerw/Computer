#include "config.h"
#include <list>
#include <sstream>

namespace sylar{

  ConfigVarBase::ptr Config::LookUpBase(const std::string& name){
      RWMutexType::ReadLockGuard lock_guard(GetMutex());
      auto it = GetDatas().find(name);
      return (it == GetDatas().end()) ? nullptr : it->second;
  }
  
  //将"A.B", 10
  //转为A:
  //      B: 10
  //便于以层次的形式遍历解析
  static void ListAllMember(const std::string& prefix,const YAML::Node& node,
                            std::list<std::pair<std::string,const YAML::Node>>& output){
          //检查是否有非法字符
      if(prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._012345678") != std::string::npos){
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Config invalid name:" << prefix << " : " << node;
            return;
      }
      output.push_back(std::make_pair(prefix,node));
      if(node.IsMap()){
        for(auto it=node.begin(); it != node.end(); ++it)
          ListAllMember(prefix.empty() ? it->first.Scalar() : prefix+"."+it->first.Scalar(),it->second,output);
      }
  }
  void Config::LoadFromYaml(const YAML::Node& root){
    std::list<std::pair<std::string,const YAML::Node> > all_nodes;
    ListAllMember("",root,all_nodes);
    for(auto& it: all_nodes){
      std::string key = it.first;
      if(key.empty())
        continue;
      //转为小写
      std::transform(key.begin(),key.end(),key.begin(),::tolower);

      //在s_datas类创建一个配置项的基类，然后再填充内容
      ConfigVarBase::ptr var = LookUpBase(key);
      if(var){
        if(it.second.IsScalar()){   //node为字符串类型
            var->fromString(it.second.Scalar());
        }else{
          //将其转为string
          std::stringstream ss;
          ss << it.second;
          var->fromString(ss.str());
        }
      }
    }
  }
  void Config::Visit(std::function<void(ConfigVarBase::ptr)> cb){
    RWMutexType::ReadLockGuard lock_guard(GetMutex());
    ConfigVarMap& m = GetDatas();
    //回调用户自定义的处理方法
    for(auto it=m.begin();it!=m.end();++it)
        cb(it->second);
  }

  

}
