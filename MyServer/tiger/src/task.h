
#pragma once

#include<functional>

namespace tiger{

  enum class TaskState{
    RUNNABLE,
    BLOCK,
    DONE,
  };

  //将state转为字符串
  const char* GetTaskStateName(TaskState state);

  typedef std::function<void()> TaskFunc;






}
