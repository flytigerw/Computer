

#define _GNU_SOURCE
#include <signal.h>
#include "tlpi_hdr.h"
#include "signal_functions.h" 


//count deliveries of each signal 
static int sigCnt[NSIG];

//SIGINT标志位
static volatile sig_atomic_t gotSigint = 0;

//若sig为SIGINT--->标志位置位，否则统计次数
static void 
handler(int sig){
  if(sig == SIGINT)
    gotSigint = 1;          
  else 
    sigCnt[sig]++;           
}

int main(int argc, char* argv[]){
 
  //show PID
  printf("%s PID is %ld\n",argv[0],(long)getpid());
  
  //install the same handler for all signals 
  for(int i=1; i<NSIG; i++)
    (void)signal(i,handler);
  
  //if a sleep time is specified 
  if(argc>1){
    int numSecs = getInt(argv[1],GN_GT_0,NULL);

    //temporarily block  all signals 
    sigset_t block;
    sigfillset(&block);
    if(sigprocmask(SIG_SETMASK,&block,NULL) == -1)
      errExit("sigprocmask");

    //sleep
    printf("%s:is sleeping for %d seconds\n",argv[0],numSecs);
    sleep(numSecs);

    //display the pending signals 
    sigset_t pending;
    if(sigpending(&pending) == -1)
      errExit("sigpending");
    printf("%s:pending signals are:\n",argv[0]);
    printSigset(stdout,"\t\t",&pending);

    //unblock all signals 
    sigset_t empty;
    sigemptyset(&empty);
    if(sigprocmask(SIG_SETMASK,&empty,NULL) == -1)
      errExit("sigprocmask");
  }
   
   //loop until catch SIGINT 
   while(!gotSigint)
     continue;

   //display number of signals received
   for(int i=1; i < NSIG; i++)
     if(sigCnt[i] != 0)
       printf("%s:signal %d caught %d time%s\n",argv[0],i,sigCnt[i], sigCnt[i]==1 ? "":"s");

  exit(EXIT_SUCCESS);
}

