

#include <signal.h>
#include "tlpi_hdr.h"
#include "curr_time.h"
#include "print_wait_status.h"
#include <sys/wait.h>

//Number of children started but not yet waited on 
static volatile int numAliveChildren = 0;
static void 
sigchildHandler(int sig){
  int savedErrno = errno ;
  printf("%s handler:Caught SIGCHILD\n",currTime("%T"));

  //call waitpid to deal zombies 
  pid_t childPid;
  int status;
  while((childPid=waitpid(-1,&status,WNOHANG)) > 0 ){
    printf("%s handler:Reaped child%ld-",currTime("%T"),(long)childPid);
    printWaitStatus(NULL,status);
    numAliveChildren--;
  }
  if(childPid == -1 && errno != ECHILD)
    errMsg("waitpid");
  sleep(5);  //Artifically 
  printf("%s handler:returning\n",currTime("%T"));
  errno=savedErrno;
}

int main(int argc,char* argv[]){
   setbuf(stdout,NULL);

   if(argc<2)
     usageErr("%s:child-sleep-time\n",argv[0]);
   //install handler for SIGCHILD
   numAliveChildren = argc-1;
   struct sigaction sa;
   sa.sa_handler=sigchildHandler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags=0;
   if(sigaction(SIGCHLD,&sa,NULL) == -1)
     errExit("sigaction");
   //block SIGCHLD to prevent its delivery when executing key codes
   sigset_t blockMask;
   sigemptyset(&blockMask);
   sigaddset(&blockMask,SIGCHLD);
   if(sigprocmask(SIG_SETMASK,&blockMask,NULL) == -1)
     errExit("sigprocmask");

   for(int i=1;i<argc;i++){
     switch(fork()){
       case -1:
         errExit("fork");
       case 0:
         //child sleep and exit 
         sleep(getInt(argv[i],GN_NONNEG,"child-sleep-time"));
         printf("%s Child %d (PID=%ld) existing\n",currTime("%T"),i,(long)getpid());
         _exit(EXIT_SUCCESS);
       default:
         break;
     }
   }

   //Wait for SIGCHILD until all children are dead 
   sigset_t emptyMask;
   sigemptyset(&emptyMask);
   int sigCnt=0;
   while(numAliveChildren > 0){
     if(sigsuspend(&emptyMask) == -1 && errno != EINTR)
       errExit("sigsuspend");
     sigCnt++;
   }

   printf("%s All %d children have terminated;SIGCHLD was caught %d times\n",currTime("%T"),argc-1,sigCnt);

   exit(EXIT_SUCCESS);
}
