

//参数: targetProcessPID sigNum SIGXX 

#include <signal.h>
#include "tlpi_hdr.h"

int main(int argc,char* argv[]){

  if(argc < 4)
    usageErr("arguments are not enough");


  pid_t pid = getLong(argv[1],0,"PID");
  int sigNum = getInt(argv[2],GN_GT_0,"sigNum");
  int sig = getInt(argv[3],0,"SIG");

  //send signals to receiver
  printf("%s:sended signal %d to process %ld %dtimes\n",argv[0],sig,(long)pid,sigNum);

  for(int i=0; i<sigNum; i++){
    if(kill(pid,sig) == -1)
      errExit("kill");
  }

  //if fourth argument was specified ,send that signal 
  if(argc>4)
    if(kill(pid,getInt(argv[4],0,"sig-num-2")) == -1)
      errExit("kill");

  printf("%s:exiting\n",argv[0]);

    
  exit(EXIT_SUCCESS);
}
