
#include <signal.h>
#include "tlpi_hdr.h"



//为SIGINT和SIGQUIT建立相同handler

static void 
newHandler(int sig){   //sig用于判断哪个信号触发了该handler
  static int count =0;

  if(sig == SIGINT){
    count++;
    printf("Caught SIGINT (%d)\n",count);
    return ;
  }

  //if the sig is SIGQUIT
  printf("Caught SIGQUIT\n");
  exit(EXIT_SUCCESS);
}
int main(int argc,char* argv[]){
  
  if(signal(SIGINT,newHandler) == SIG_ERR)
    errExit("signal");
  if(signal(SIGQUIT,newHandler) == SIG_ERR)
    errExit("signal");
        

  for(;;)
    pause();         //block forever -- to catch signal 


}
