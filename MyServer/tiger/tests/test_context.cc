#include "../src/MyGo/context/context.h"
#include "../src/log.h"

tiger::Logger::ptr g_logger = TIGER_LOG_ROOT;


void test_fun(intptr_t m){
   TIGER_LOG_INFO(g_logger) << "context swapped_in";
   TIGER_LOG_INFO(g_logger) << "hello wrold";
   ((tiger::Context*)m)->swap_out();
}



int main(){
  TIGER_LOG_INFO(g_logger) << "test_begin";
  tiger::Context c(&test_fun,0,1024*10);
  c.swap_in();
  TIGER_LOG_INFO(g_logger) << "context swapped_out";
  return 0;
}


