
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "tlpi_hdr.h"



static char* str2 ;             //对应argv[2],在main中获得，在handler中使用，故采用static
static int handled=0;           //count number of calls to another
static void 
handler (int sig){
  //加密str2 
  crypt(str2,"xx");
  handled++;
}
int main(int argc,char* argv[]){
  
  if(argc != 3)
    usageErr("arguments are not enough");
   
    str2 = argv[2];
    
    //加密str1,将结果复制到独立缓冲区中
    char* cr1 = strdup(crypt(argv[1],"xx"));
    if(cr1 == NULL)
      errExit("strdup");
  
    //为SIGINT install handler
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handler;
  if(sigaction(SIGINT,&sa,NULL) == -1)
    errExit("sigaction");

  //Repeatedly call crypt() using argv[1]
  //If interrupted by a signal handler,then the static storage returned by crypt() will be overriten by the 
  //results of encrypting argv[2] and strcmp will detect a mismatch 
  for(int i=1,j=0;;i++){
    if(strcmp(crypt(argv[1],"xx"),cr1) != 0 ){
      j++;
      printf("Mismatch on call %d (mismatch=%d handled=%d)\n",i,j,handled);
    }
  }
}
