
//演示SIGKILL无法kill僵尸进程
#include <signal.h>
#include <libgen.h>
#include "tlpi_hdr.h"

#define CMD_SIZE 200

int main(int argc, char* argv[]){

  setbuf(stdout,NULL);

  printf("Parent PID=%ld\n",(long)getpid());

  int childPid;
  char cmd[CMD_SIZE];
  switch(childPid = fork()){
    case -1:
      errExit("fork");
    case 0:
      printf("Child (PID=%ld) existing\n",(long)getpid());
      _exit(EXIT_SUCCESS);
    default:
      //give child a chance to exit 
      sleep(3);
      //execute cmd to show zombie
      snprintf(cmd,CMD_SIZE,"ps | grep %s",basename(argv[0]));
      cmd[CMD_SIZE-1]='\0';   
      system(cmd);

      //send SIGKILL to zombie and watch if they will be killed
      if(kill(childPid,SIGKILL) == -1)
        errExit("kill");
      sleep(3);            //give child a chance to react to signal 
      printf("After sending SIGKILL to zombie (PID=%ld):\n",(long)getpid());
      system(cmd);

      exit(EXIT_SUCCESS);
      
  }
}
