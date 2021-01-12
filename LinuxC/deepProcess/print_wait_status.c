
#define _GNU_SOURCE
#include <string.h>
#include <sys/stat.h>
#include "tlpi_hdr.h"
#include "print_wait_status.h"

void 
printWaitStatus(const char* msg, int status){
  //print msg first
  if(msg != NULL)
    printf("%s",msg);

  //通过Wxxx(status)显示子进程的状态信息
  if(WIFEXITED(status)){
    printf("child exited, status=%d\n",WEXITSTATUS(status));
  }else if(WIFSIGNALED(status)){
    printf("child killed by signal %d(%s)\n",WTERMSIG(status),strsignal(WTERMSIG(status)));
  }else if(WIFSTOPPED(status)){
    printf("child stopped  by signal %d(%s)\n",WSTOPSIG(status),strsignal(WSTOPSIG(status)));
  
#ifdef WIFCONTINUED 
  }else if(WIFCONTINUED(status)){
    printf("child continued\n");
 #endif
  }else{
    printf("what happend to this child?(status=%x)\n",(unsigned int)status);
  }
}
