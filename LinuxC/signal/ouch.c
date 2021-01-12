
#include <signal.h>
#include "tlpi_hdr.h"


//为SIGNT设置handler

static void 
newHandler(int sig){
  printf("Ouch\n");
}
int main(int argc,char* argv[]){
  
  if(signal(SIGINT,newHandler) == SIG_ERR)
    errExit("signal");

  for(int i=0;;i++){
    printf("%d\n",i);
    sleep(3);
  }
  exit(EXIT_SUCCESS);
}
