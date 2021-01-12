
#include <signal.h>
#include "tlpi_hdr.h"
#include "curr_time.h"

#define SYNC_SIG SIGUSR1   //Synchronization signal 

static void 
handler(int sig){
  //do nothing and return 
}

int main(int argc,char* argv[]){
  //先阻塞SYNC_SIG,然后fork() --> 父进程则解除阻塞，挂起等待   子进程完成任务，发送SYNC_SIG  
  
  sigset_t block , preMask;
  //block signal 
  sigemptyset(&block);
  sigaddset(&block,SYNC_SIG);
  if(sigprocmask(SIG_BLOCK,&block,&preMask) == -1)
    errExit("sigprocmask");
  
  //install handler 
  struct sigaction sa;
  sa.sa_handler=handler;
  sa.sa_flags=SA_RESTART;
  sigemptyset(&sa.sa_mask);
  if(sigaction(SYNC_SIG,&sa,NULL) == -1)
    errExit("sigaction");
  
  int childPid;
  switch(childPid = fork()){
    case -1:
      errExit("fork");
    case 0:
      //child do the task and send signal 
      printf("[%s,%ld] Child started doing some work\n",currTime("%T"),(long)getpid());
      sleep(2);
      printf("[%s,%ld] Child about to signal parent \n",currTime("%T"),(long)getpid());
      //send signal 
      if(kill(getppid(),SYNC_SIG) == -1)
        errExit("kill");
      _exit(EXIT_SUCCESS);
    default:
      printf("[%s,%ld] Parent about to wait for signal\n",currTime("%T"),(long)getpid());
      sigset_t empty;
      sigemptyset(&empty);
      if(sigsuspend(&empty) == -1 && errno != EINTR)
        errExit("sigsuspend");
      printf("[%s,%ld] Parent got signal\n",currTime("%T"),(long)getpid());
      //restore previous mask 
      if(sigprocmask(SIG_SETMASK,&preMask,NULL) == -1)
        errExit("sigprocmask");

      exit(EXIT_SUCCESS);

  }


}
