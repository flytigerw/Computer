#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"
#include "become_daemon.h"

int becomeDaemon(int flags){
  switch(fork()){  //Beceome BG process
    case -1:
      return -1;
    case 0:
      break;
    default:
      _exit(EXIT_SUCCESS);
  }
  if(setsid() == -1)  //Start a new session and free itself of any association with controlling terminal
    return -1;
  
  //Ensure we are not session leader
  switch(fork()){  
    case -1:
      return -1;
    case 0:
      break;
    default:
      _exit(EXIT_SUCCESS);
  }
  
  if(!(flags & BD_NO_UMASK0))
    umask(0);                 //Clear file mode creation mask 
  if(!(flags & BD_NO_CHIDIR))
    chdir("/");               //Change to dir /

  int maxfd;
  if(!(flags & BD_NO_CLOSE_FILES)){  //Close all open files
      maxfd=sysconf(_SC_OPEN_MAX);
      if(maxfd == -1)                     //Limit is underminate
        maxfd = BD_MAX_CLOSE;
      for(int fd=0;fd<maxfd;fd++)
        close(fd);
  }
  
  int fd;
  if(!(flags & BD_NO_REOPEN_STD_FDS)){
    close(STDIN_FILENO);            //Reopen stdFds to /dev/null 
    fd = open("/dev/null",O_RDWR);
    if(fd != STDIN_FILENO)
      return -1;
    if(dup2(STDIN_FILENO,STDOUT_FILENO) != STDOUT_FILENO)
      return -1;
    if(dup2(STDIN_FILENO,STDERR_FILENO) != STDOUT_FILENO)
      return -1;
  }
  return 0;
}
