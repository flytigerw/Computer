

#include "curr_time.h"
#include "tlpi_hdr.h"


//Use pipe as a method of synchronization between related processes
int main(int argc, char* argv[]){
  
  //用参数中的睡眠时间模拟child 工作时间
  if(argc < 2)
    usageErr("%s sleep-time...",argv[0]);

  setbuf(stdout,NULL);
  printf("%s parent started\n",currTime("%T"));

  //create pipes 
  int fds[2];
  if(pipe(fds) == -1)
    errExit("pipe");

  //fork children 
  for(int i=1;i<=argc;i++){
    switch(fork()){
      case -1:
        errExit("fork");
      case 0:
        //close unused read fd 
        if(close(fds[0]) == -1)
          errExit("close");

        //Do own work 
        sleep(getInt(argv[i],GN_NONNEG,"sleep-time"));

        //inform parent by close write fd 
        printf("%s child %d (PID=%ld) closed pipe\n",currTime("%T"),i,(long)getpid());

        //Do other work ...
        _exit(EXIT_SUCCESS);
      default:  //parent loops to create next child
        break;
    }
  }
  //close unused write fd 
  if(close(fds[1]) == -1)
    errExit("close");

  //block waiting for child to close write fds 
  int dummy;
  if(read(fds[0],&dummy,1) != 0)
     fatal("Parent didnt get EOF") ;

  printf("Parent ready to go\n");

  //parent can do other  work 
  exit(EXIT_SUCCESS);
}
