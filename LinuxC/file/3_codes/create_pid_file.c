

#include <sys/stat.h>
#include <fcntl.h>
#include "region_locking.h"
#include "create_pid_file.h"
#include "tlpi_hdr.h"

#define BUF_SIZE 100    //Large enough to hold maimum PID as string 

/* Open/Create the 'pidFile',lock it,optionally set close-on-exec flag for that fd,write PID into the file and return * the fd
 * The caller is responsible for deleteing 'pidFile' before process termination
 * */


int createPidFile(const char* progName,const char* pidFile,int flags){
  int fd = open(pidFile,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
  if(fd == -1)
    errExit("Could not open PID file %s",pidFile);

  if(flags & CPF_CLOEXEC){
    flags = fcntl(fd,F_GETFD);
    if(flags == -1)
      errExit("Couldn't get flags for PID file %s",pidFile);

    flags |= FD_CLOEXEC;
    if(fcntl(fd,F_SETFD,flags) == -1)
      errExit("Couldn't set flags for PID file %s",pidFile);
  }

  if(lockRegion(fd,F_WRLCK,SEEK_SET,0,0) == -1){
    if(errno == EAGAIN || errno == EACCES)
      fatal("PID file %s is loced.probably %s is already running",pidFile,progName);
    else 
      errExit("Unable to lock PID file%s",pidFile);
  }
  
  //It erases any previous string in the lock file 
  //The last instance may have failed to delete the file,perhaps because of system crash
  if(ftruncate(fd,0) == -1)
    errExit("Counldn't truncate PID file %s",pidFile);
  
  char buf[BUF_SIZE];
  snprintf(buf,BUF_SIZE,"%ld\n",(long)getpid());
  //write PID 
  if(write(fd,buf,strlen(buf)) != (ssize_t)strlen(buf))
    fatal("Writing to PID file '%s' ",pidFile);
  return fd;
}
