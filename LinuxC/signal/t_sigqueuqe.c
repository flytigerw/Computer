

//arguments: targetProcessID, SIGXX, int data  numSigs 


#define _POSIX_C_SOURCE 199309
#include <signal.h>
#include "tlpi_hdr.h"

int main(int argc,char* argv[]){
  if(argc < 4)
    usageErr("arguments are not enough");

  //Display UID,PID,so that they can be compared with corresponding fields of siginfo_t argument  supplied to the handler supplied to receiving process 
  printf("%s:PID is %ld,UID is %ld\n",argv[0],(long)getpid(),(long)getuid());

  //parse the arguments 
  int sig = getInt(argv[2],0,"sig-num");
  int sigData = getInt(argv[3],GN_ANY_BASE,"data");
  int numSigs = argc>4 ? getInt(argv[4],GN_GT_0,"num-sigs") : 1;
  union sigval sv;
  //send RTsignals 
  for(int i=0; i<numSigs; i++){
      sv.sival_int = i+sigData;
      if(sigqueue(getLong(argv[1],0,"pid"),sig,sv) == -1)
        errExit("sigqueue");
  }


  exit(EXIT_SUCCESS);
}

