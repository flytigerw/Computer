#include<signal.h>
#include"tlpi_hdr.h"

static void tstHandler(int sig){

  //save errno in case we change it 
  int savedErrno = errno; 
  printf("Caught SIGTSTP\n");
  //reset dispostion of SIGTSTP to default action SIG_DFL
  if(signal(SIGTSTP,SIG_DFL) == -1)
    errExit("SIG_DFL");
  //generate furthur SIGTSTP
  raise(SIGTSTP);

  //Unblock SIGTSTP and the pending SIGTSTP immediately suspends the program
  sigset_t tstpMask,prevMask;
  sigemptyset(&tstpMask);
  sigaddset(&tstpMask,SIGTSTP);
  if(sigprocmask(SIG_UNBLOCK,&tstpMask,&prevMask) == -1)
    errExit("sigprocmask");

  //execution resumes here afer SIGCONT
  //Reblock the SIGTSTP
  if(sigprocmask(SIG_SETMASK,&prevMask,NULL) == -1)
    errExit("sigprocmask");

  //reestablish handler 
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = tstHandler;
  if(sigaction(SIGTSTP,&sa,NULL) == -1)
    errExit("sigaction");

  printf("Exiting SIGTSTP handler\n");
  errno = savedErrno;
}

int main(int argc,char* argv[]){
   struct sigaction sa;
   //Only establish handler for SIGTSTP if it is not being ignored
   if(sigaction(SIGTSTP,NULL,&sa) == -1)
     errExit("sigaction");

   if(sa.sa_handler != SIG_IGN){
     sigemptyset(&sa.sa_mask);
     sa.sa_flags = SA_RESTART;
     sa.sa_handler = tstHandler;
     if(sigaction(SIGTSTP,&sa,NULL) == -1)
       errExit("sigaction");
   }
   //wait for signals
   for(;;){
     pause();
     printf("main\n");
   }
}
