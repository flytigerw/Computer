
#define _GNU_SOURCE
#include <string.h>
#include <signal.h>
#include "signal_functions.h"
#include "tlpi_hdr.h"

//Print list of signals within a signal set to a file
void 
printSigset(FILE* of, const char* prefix, const sigset_t* sigset){
  int cnt =0 ;
  for(int sig=1; sig<NSIG; sig++){    //NSIG为信号编号上限
    if(sigismember(sigset,sig)){
        cnt++;
        fprintf(of,"%s%d (%s)\n",prefix,sig,strsignal(sig));
    }
  }
  if(cnt == 0)
    fprintf(of,"%s<empty signal set>\n",prefix);
}

//print mask of blocked signals of current process
int 
printSigMask(FILE* of, const char* msg){
  if(msg != NULL)
    fprintf(of,"%s",msg);
  //获取当前进程的掩码信号集合
  sigset_t mask;
  if(sigprocmask(SIG_BLOCK,NULL,&mask) == -1)
    return -1;;
  //print sigset 
  printSigset(of,"\t\t",&mask);
  return 0;
}

//print signals currently pending for this process 
int 
printPendingSigs(FILE* of,const char* msg){
  if(msg != NULL)
    fprintf(of,"%s",msg);

  sigset_t pendingSigs; 

  if(sigpending(&pendingSigs) == -1)
    return -1;

  printSigset(of,"\t\t",&pendingSigs);

  return 0;
}
