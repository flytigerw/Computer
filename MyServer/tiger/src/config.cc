#include "config.h"
#include <sstream>
#include <algorithm>

namespace tiger{
  static Logger::ptr g_logger = TIGER_LOG_LOGGER("system");
//-------------------ConfigManager--------------------
      ConfigItemBase::ptr ConfigManager::LookUpBase(const std::string& name){
        auto it = GetConfigItems().find(name);
        if(it == GetConfigItems().end())
          return  nullptr;
        return it->second;
      }
        

/* EX:
 *  system:
 *    port: 8080
 *    int_value: 12
 *    int_vec: [10, 20, 30]
 *    int_map:
 *      k1: 20
 *      k2: 30
 **/
  //简单处理
  void ConfigManager::get_all_nodes(const std::string& name,const YAML::Node node,std::vector<std::pair<std::string,YAML::Node>>& output){
        
    //检查name的合法性
    if(name.find_last_not_of("abcdeghijklmnopqrstuvwxyz"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "123456789_.") != std::string::npos){
      TIGER_LOG_INFO(g_logger) << "invalid config name:" << name;
      return;
    }
    //压入node:比如 system.port 8080
    output.push_back(std::make_pair(name,node));
    //yaml中的对象为map类型
    if(node.IsMap()){
      for(auto it=node.begin();it != node.end(); it++){  //针对每一个map node,构建出该Node的name,递归调用进入下一层
          get_all_nodes(name.empty() ? it->first.Scalar() : name+"."+it->first.Scalar(),it->second,output);
      }
    }
  }


  void ConfigManager::load_from_yaml(const std::string& file){
    YAML::Node root_node = YAML::LoadFile(file);
    std::vector<std::pair<std::string,YAML::Node>> vec;
    std::string name = "";
    get_all_nodes(name,root_node,vec);
    
    //将每个node string化，再设置ConfigItem
    for(auto& it:vec){
      name = it.first;
      
      //在map中找到该ConfigItemBase 
      ConfigItemBase::ptr x = LookUpBase(name);
      if(x){
        if(it.second.IsScalar())  //可以直接转为string
          x->from_string(it.second.Scalar());
        else{  //通过stringstream流转为string
          std::stringstream ss;
          ss << it.second;
          x->from_string(ss.str());
        }
      }
    }
  }


}
