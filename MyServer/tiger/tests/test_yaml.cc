#include <yaml-cpp/yaml.h>
#include "../src/log.h"
#include "../src/config.h"



static tiger::Logger::ptr g_logger = TIGER_LOG_ROOT;



#if 0
//加入level,使打印起来更有层次感
void print_yaml(const YAML::Node& root,int level){
  if(root.IsScalar()){
    TIGER_LOG_INFO(g_logger) << std::string(level,' ')<<"sca----" <<root.Scalar();
  }
  else if(root.IsNull()){
    TIGER_LOG_INFO(g_logger) <<std::string(level,' ')<< "nul----"<< "NULL";
  }
  else if(root.IsMap()){
    for(auto it=root.begin(); it != root.end(); ++it){
      TIGER_LOG_INFO(g_logger) << std::string(level,' ') << "map----"<<it->first <<":";
      print_yaml(it->second,++level);
      --level;
    }
  }else if(root.IsSequence()){
    for(size_t i=0;i<root.size();i++){
      TIGER_LOG_INFO(g_logger) << std::string(level,' ') << "seq----";
        print_yaml(root[i],++level);
    }
  }
}

void test_yaml(){
    TIGER_LOG_INFO(g_logger) << "yaml-test begin------:";
    //Load YAML file 
    const std::string file ="/root/MyServer/tiger/tests/test.yml"; 
    YAML::Node root_node = YAML::LoadFile(file);
    if(!root_node)
      TIGER_LOG_INFO(g_logger) << "Load yaml file:" << file << "failed";
    print_yaml(root_node,0);
    TIGER_LOG_INFO(g_logger) << "yaml-test end------";
}
#endif

void test_config(){
  //先手动创建配置项:通过configManager创建
  tiger::ConfigItem<int>::ptr c_port = tiger::ConfigManager::LookUp("system.port",8090,"系统端口");
  tiger::ConfigItem<int>::ptr c_value = tiger::ConfigManager::LookUp("system.value",20,"系统参数个数");
  tiger::ConfigItem<std::vector<int>>::ptr c_int_vec = tiger::ConfigManager::LookUp("system.int_vec",std::vector<int>{30,40,50},"系统参数vec");
  tiger::ConfigItem<std::map<std::string,int>>::ptr c_str_int_map = tiger::ConfigManager::LookUp("system.str_int_map",std::map<std::string,int>{{"k",10}},"系统map");

  //添加回调函数
  c_port->add_listener([](int& old_value,const int& new_value){
          TIGER_LOG_INFO(g_logger) << "system port is changed to " << new_value;
      });


#define XX(name,time)\
  std::cout << #time"\t:" << name->dump() << std::endl;
  XX(c_port,before);
  XX(c_value,before);
  XX(c_int_vec,before);
  XX(c_str_int_map,before);
  const std::string file ="/root/MyServer/tiger/tests/test2.yml"; 
  tiger::ConfigManager::load_from_yaml(file);
  XX(c_port,after);
  XX(c_value,after);
  XX(c_int_vec,after);
  XX(c_str_int_map,after);
#undef XX
}


void test_log() {
    TIGER_LOG_INFO(g_logger) << "hello system" << std::endl;
    std::cout << tiger::LoggerMgr::GetInstance()->to_yaml_string() << std::endl;
    const std::string file ="/root/MyServer/tiger/tests/log.yml"; 
    tiger::ConfigManager::load_from_yaml(file);
    std::cout << "=============" << std::endl;
    std::cout << tiger::LoggerMgr::GetInstance()->to_yaml_string() << std::endl;
    std::cout << "=============" << std::endl;
    g_logger->set_formatter("%d - %m%n");
    TIGER_LOG_INFO(g_logger) << g_logger->to_yaml_string() << std::endl;
}





int main(){
 // test_yaml();
 //test_config();
 test_log();
  return 0;
}
