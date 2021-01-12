

#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"
#include <sys/wait.h>

int main(int argc, char* argv[]){
  //disable buffering of stdout 
  setbuf(stdout,NULL);
  //父进程创建临时文件，然后fork,子进程修改临时文件的偏移。
  char tmp[]="/tmp/testXXXXXX";
  int fd = mkstemp(tmp);
  if(fd == -1)
    errExit("mkstemp");

  printf("Fileoffset before fork() %lld\n",(long long)lseek(fd,0,SEEK_CUR));

  //show the file stauts
  int flags = fcntl(fd,F_GETFL);
  if(flags == -1)
    errExit("fcntl");
  printf("O_APPEND flag before fork is:%s\n",(flags & O_APPEND) ? "on":"off");
  
  switch(fork()){
    case -1:
      errExit("fork");
    //child process change file offset and status
    case 0:
      if(lseek(fd,1000,SEEK_SET) == -1)
        errExit("lseek");
      flags = fcntl(fd,F_GETFL);
      if(flags == -1)
        errExit("fcntl");

      flags |= O_APPEND;
      if(fcntl(fd,F_SETFL,flags) == -1)
        errExit("fcntl");
      _exit(EXIT_SUCCESS);

    default:
      if(wait(NULL) == -1)
        errExit("wait");            //wait for child exit 
      printf("Child has exited\n");

      printf("FIle offset in parent:%lld\n",(long long)lseek(fd,0,SEEK_CUR));

      if((flags=fcntl(fd,F_GETFL)) == -1)
        errExit("fcntl");

      printf("O_APPEND flag in parent is:%s",(flags & O_APPEND) ? "on":"off");
      exit(EXIT_SUCCESS);
  }


}
