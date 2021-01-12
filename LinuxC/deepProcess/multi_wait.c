
#include <sys/wait.h>
#include "tlpi_hdr.h"
#include "curr_time.h"
#include <time.h>

int main(int argc, char* argv[]){
  
  if(argc < 2)
    usageErr("%s:sleeptime....\n",argv[0]);

  setbuf(stdout,NULL);   //disable buffering of stdout

  //create childs 
  for(int i=1; i<argc;++i){
    switch(fork()){
      case -1:
        errExit("fork");
      case 0:
        printf("[%s] child %d started with PID %ld,sleeping %s secs\n",currTime("%T"),i,(long)getpid(),argv[i]);
        sleep(getInt(argv[i],GN_NONNEG,"sleep-time"));
        _exit(EXIT_SUCCESS);
      default:
        break;
    }
  }
  //wait childs
  int numDead=0;
  for(;;){
    int childPid=wait(NULL);
    if(childPid == -1){
      if(errno == ECHILD){
        printf("No more childern bye!\n");
        exit(EXIT_SUCCESS);
      }else{
        errExit("wait");
      }
    }
    numDead++;
    printf("[%s] wait() returned child PID %ld (numDead=%d)\n",currTime("%T"),(long)childPid,numDead);
  }

}
