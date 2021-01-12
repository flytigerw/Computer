#define _GNU_SOURCE
#include <string.h>
#include <signal.h>
#include "tlpi_hdr.h"

static void handler(int sig){
  printf("PID=%ld:caught signal %d(%s)\n",(long)getpid(),sig,strsignal(sig));
}

int main(int argc,char* argv[]){
  if(argc < 2)
    usageErr("%s {s|p}...\n",argv[0]);

  setbuf(stdout,NULL);
  //install handler for SIGHUP and SIGCONT
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handler;
  if(sigaction(SIGHUP,&sa,NULL) == -1)
    errExit("sigaction");
  if(sigaction(SIGCONT,&sa,NULL) == -1)
    errExit("sigaction");

  printf("parent: PID=%ld, PPID=%ld, PGID=%ld, SID=%ld\n",
          (long)getpid(),(long)getppid(),(long)getpgrp(),(long)getsid(0));


  //Create one child for each CMD_LINE argument
  for(int j=1;j<argc;j++){
    switch(fork()){
      case -1:
        errExit("fork");
      case 0:
       printf("Child: PID=%ld, PPID=%ld, PGID=%ld, SID=%ld\n",
          (long)getpid(),(long)getppid(),(long)getpgrp(),(long)getsid(0));
        
       if(argv[j][0] == 's'){ //Stop via signal
          printf("PID=%ld stopping\n",(long)getpid());
          raise(SIGSTOP);
       }else{    //wait for signal
         alarm(60);
         printf("PID=%ld pausing\n",(long)getpid());
         pause();
       }
       _exit(EXIT_SUCCESS);
      default:
       break;
    }
  }
  sleep(3);          //Give children a chance to start
  printf("parent exitsing\n");
  exit(EXIT_SUCCESS);
}
