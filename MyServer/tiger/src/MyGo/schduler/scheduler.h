

#pragma once 


#include "processor.h"

namespace tiger{



  //一个Scheduler管理多个Processor ---> 为每个Processor分配线程并运行
  
  class Scheduler{




    public:
      //当前正在运行的Processor
      static Processor* GetCurrentProcessor();

  };




}
