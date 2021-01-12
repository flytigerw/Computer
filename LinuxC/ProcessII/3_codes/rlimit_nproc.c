#include <sys/resource.h>
#include "tlpi_hdr.h"
#include "print_rlimit.h"

int main(int argc,char* argv[]){
  if(argc<2 || argc>3 )
    usageErr("%s soft-limit[hardlimit]\n",argv[0]);
  
  printRlimit("Initial maximus process limits: ",RLIMIT_NPROC);

  //set new process limits
  struct rlimit rl;
  rl.rlim_cur = (argv[1][0] == 'i') ? RLIM_INFINITY:
                          getInt(argv[1],0,"softlimit");
  rl.rlim_max = (argv[2][0] == 'i') ? RLIM_INFINITY:
                          getInt(argv[2],0,"hardlimit");
  if(setrlimit(RLIMIT_NPROC,&rl) == -1)
    errExit("setrlimit");

  printRlimit("New maximux process limits:  ",RLIMIT_NPROC);
  
  //Create as many as children as possible
  pid_t childPid;
  for(int j=1;;j++){
    switch(childPid = fork()){
      case -1:
        errExit("fork");
      case 0:
        _exit(EXIT_SUCCESS);
      default:
        printf("Child %d (PID=%ld) started\n",j,(long)childPid);
        break;
    }
  }
}
