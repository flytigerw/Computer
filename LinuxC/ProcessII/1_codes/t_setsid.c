
#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

int main(int argc,char* argv[]){
  //Exit if parent or on error
  if(fork() != 0)
    _exit(EXIT_SUCCESS);
  //child process
  if(setsid() == -1)
    errExit("setsid");
  printf("PID=%ld,SID=%ld",(long)getpid(),(long)getsid(0));
  if(open("/dev/tty",O_RDWR) == -1)
    errExit("open /dev/tty");
}
