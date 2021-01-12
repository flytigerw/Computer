#define _GNU_SOURCE  //Get strsignal() declaration from <string.h>

#include <string.h>
#include <signal.h>
#include "tlpi_hdr.h"

static void hander(int sig){
  printf("PID %ld:caught signal %2d (%s)\n",(long)getpid(),sig,strsignal(sig));
}

int main(int argc, char* argv[]){
   if(argc<2 || strcmp(argv[1],"--help") == 0)
         usageErr("%s,{d|s}..[> sig.log 2>&1]\n",argv[0]);

   setbuf(stdout,NULL);

   pid_t parentPid,childPid;
   parentPid = getpid();
   printf("PID of the parent process is:  %ld\n",(long)parentPid);
   printf("Foreground process group ID is:%ld\n",(long)tcgetpgrp(STDIN_FILENO));

   for(int j=1;j<argc;j++){   //Create child process
     switch(childPid=fork()){
        case -1:
          errExit("fork");
        case 0:
          if(argv[j][0] == 'd')          //to different group
            if(setpgid(0,0) == -1)
              errExit("setpgid");
          //Install handler for SIGHUP
          struct sigaction sa;
          sigemptyset(&sa.sa_mask);
          sa.sa_flags = 0;
          sa.sa_handler = hander;
          if(sigaction(SIGHUP,&sa,NULL) == -1)
            errExit("sigaction");
          break;       //child exits loop
        default:
          continue;
     }
   }
   //All process fall through to here
   alarm(60);             //ensure each process eventually terminates
   printf("PID=%ld PGID=%ld\n",(long)getpid(),(long)getpgrp());

   for(;;)
     pause();
}
