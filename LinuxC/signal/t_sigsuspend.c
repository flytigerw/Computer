

#define  _GNU_SOURCE
#include <signal.h>
#include "tlpi_hdr.h"
#include "signal_functions.h"
#include <time.h>
#include <string.h>

static volatile sig_atomic_t gotSigQuit = 0;

static void 
handler(int sig){
  printf("Catch signal %d(%s)",sig,strsignal(sig));
  if(sig == SIGQUIT)
    gotSigQuit = 1;
}

int main(int argc, char* argv[]){
  printSigMask(stdout,"Initial SIGMASK is:\n");

  //block SIGINT,SIGQUIT 
  sigset_t blockMask, preMask;
  sigemptyset(&blockMask);
  sigaddset(&blockMask,SIGINT);
  sigaddset(&blockMask,SIGQUIT);
   
  if(sigprocmask(SIG_BLOCK,&blockMask,&preMask) == -1)
    errExit("sigprocmask");

  //install the same handler for SIGINT and SIGQUIT 
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_handler=handler;
  sa.sa_flags = 0;
  if(sigaction(SIGINT,&sa,NULL) == -1)
    errExit("sigaction1");
  if(sigaction(SIGQUIT,&sa,NULL) == -1)
    errExit("sigaction1");

  //loop until get SIGQUIT 
  for(int i=1; !gotSigQuit; i++){
    printf(" ======loop %d\n",i);
    //simulate a critical section by delaying a few secondes 
    printSigMask(stdout,"Sarting critical section,signal mask is:\n");

    for(time_t startTime= time(NULL);time(NULL) < startTime+4;)
      continue;

    printPendingSigs(stdout,"Before sigsuspend() - pending signals are:\n");

    //unblock signal and suspend until signals arrive 
    if(sigsuspend(&preMask) == -1 && errno != EINTR)
      errExit("sigsuspend");
  }
  
  //resotre signal mask 
  if(sigprocmask(SIG_SETMASK,&preMask,NULL) == -1 )
    errExit("sigprocmask");

  printSigMask(stdout,"===== Exited loop\n restore signal mask to:\n");

  exit(EXIT_SUCCESS);
}
