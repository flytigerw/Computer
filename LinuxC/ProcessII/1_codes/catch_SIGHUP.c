#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <signal.h>
#include "tlpi_hdr.h"

static void handler(int sig){

}

int main(int argc,char* argv[]){
  //Make stdout unbuffered
  setbuf(stdout,NULL);

  //set handler for SIGHUP
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handler;
  if(sigaction(SIGHUP,&sa,NULL) == -1)
    errExit("sigaction");

  pid_t childPid = fork();
  if(childPid == -1)
    errExit("fork");
  if(childPid == 0 && argc>1)
    if(setpgid(0,0) == -1)          //Move to a new process group
      errExit("setpgid");

  printf("PID=%ld; PPID=%ld; SID=%ld\n",(long)getpid(),(long)gerppid(),(long)getpgrp(),(long)getsid());

  alarm(60);    //An unhandled SIGALRM ensures this process will die if nothing else terminates it 
  
  for(;;){   //wait for signals
    pause();
    printf("%ld:caught SIGHUP\n",(long)getpid());
  }
}
