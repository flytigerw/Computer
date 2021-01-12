
#include <fcntl.h>
#include "tlpi_hdr.h"

int main(int argc, char* argv[]){
  if(argc>1){
    int flags = fcntl(STDOUT_FILENO,F_GETFD);
    if(flags == -1)
      errExit("fcntl-F_GETFD");
    flags |= FD_CLOEXEC;
    if(fcntl(STDOUT_FILENO,F_SETFD,flags)==-1)
      errExit("fcntl-F-F_SETFD");
  }

  execlp("ls","ls","-l",argv[0],(char*)NULL);

  errExit("execlp");  
}
