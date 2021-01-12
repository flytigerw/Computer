

#include "../src/log.h"
#include "../src/macro.h"
void test_logger(){
  tiger::Logger::ptr g_logger(new tiger::Logger("system"));
  g_logger->add_appender(tiger::LogAppender::ptr(new tiger::StdoutLogAppender));
  TIGER_LOG_INFO(g_logger) << "hello world";
  g_logger->set_formatter("%d{%Y-%m-%D %H:%M:%S}%T%f%T%l%T%m%n");
  g_logger->sync_fomatter();
  TIGER_LOG_DEBUG(g_logger) << "hello my world";

  auto  root_logger = tiger::LoggerMgr::GetInstance()->get_root_logger();
  TIGER_LOG_DEBUG(root_logger) << "hello your world";

  auto my_loger = tiger::LoggerMgr::GetInstance()->LookUp("mylogger");
  TIGER_LOG_DEBUG(my_loger) << "hello his world";
  
}



int main(){
 // test_logger();
  MY_ASSERT(0);
  return 0;
}

