
#include <signal.h>
#include "tlpi_hdr.h"

//argv[1]为sig,argv[2]为pid

int main(int argc,char* argv[]){
  if(argc != 3)
    usageErr("arguments are not enhough");

  int sig = getInt(argv[1],0,"sig-num");
  int pid=getLong(argv[2],0,"pid");

  int s = kill(pid,sig);

  if(sig != 0){     
     if(s == -1)
       errExit("kill");
  }else{  //NULL signal, process existence check

    if(s == 0){
       printf("Process exists and we can send a sig to it\n");
    }else{
     if(errno == EPERM)
       printf("Process exists but we don't have permission to sned sig to it\n");
      else if(errno == ESRCH)
      printf("Process doesnt esist");
      else 
        errExit("kill");
    }
  }
}
