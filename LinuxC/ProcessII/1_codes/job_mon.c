#define _GNU_SOURCE     //Get the declaration of strsignal() frong <string.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

static int cmdPos;            //CMD position in pipeline

//Handler for various signals
static void handler(int sig){
  if(getpid() == getpgrp()) //The PGRP leader
    fprintf(stderr,"Terminal FGRP: %ld\n",(long)tcgetpgrp(STDERR_FILENO));
  fprintf(stderr,"Process %ld(%d) receive signal %d(%s)\n",(long)getpid(),cmdPos,sig,strsignal(sig));

  //If we catch SIGSTP,it won't actually stop us. Therefore we raise SIGSTOP so we actually get stopped
  if(sig == SIGTSTP)
    raise(SIGSTOP);
}

int main(int argc,char* argv[]){
   //install handler for various signals 
   struct sigaction sa;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_RESTART;
   sa.sa_handler = handler;
   if(sigaction(SIGINT,&sa,NULL) == -1)
     errExit("sigaction");
   if(sigaction(SIGTSTP,&sa,NULL) == -1)
     errExit("sigaction");
   if(sigaction(SIGCONT,&sa,NULL) == -1)
     errExit("sigaction");
   
   //If stdin is a terminal,this is the first process in pipeline:print a heading and initialize the message to be sent down pipe
   if(isatty(STDIN_FILENO)){
     fprintf(stderr,"Terminal FGRP:%ld\n",(long)tcgetpgrp(STDIN_FILENO));
     fprintf(stderr,"CMD PID PPID PGRP  SID\n");
     cmdPos = 0;
   }else{  //Not first in pipeline,so read message from pipe
     if(read(STDIN_FILENO,&cmdPos,sizeof(cmdPos)) <= 0)
       fatal("read got EOF or error");
   }
   cmdPos++;
   fprintf(stderr,"%4d   %5ld %5ld %5ld\n",cmdPos,(long)getpid(),(long)getppid(),(long)getpgrp(),(long)getsid(0));

   if(!isatty(STDOUT_FILENO)) //not tty --> a pipe
     if(write(STDOUT_FILENO,&cmdPos,sizeof(cmdPos)) == -1)
       errMsg("write");

   //wait for signals 
   for(;;){
     pause();
   }
}
