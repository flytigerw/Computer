#include <sys/wait.h>
#include "tlpi_hdr.h" 

#define BUF_SIZE 10

int main(int argc,char* argv[]){
  if(argc != 2)
    usageErr("%s string\n",argv[0]);

  int pfds[2];
  if(pipe(pfds) == -1)
    errExit("pipe");
  
  ssize_t numRead;
  char buf[BUF_SIZE];
  switch(fork()){
    case -1:
      errExit("fork");
    case 0:
      //child reads from pipe 
      if(close(pfds[1]) == -1)
        errExit("close - child");

      //read data from pipe and echo on stdout
      for(;;){
        numRead = read(pfds[0],buf,BUF_SIZE);
        if(numRead == -1)
          errExit("read");
        if(numRead == 0)
          break;                 //end of file 
        if(write(STDOUT_FILENO,buf,numRead) != numRead)
          fatal("child - partial/failed write");
      }
      write(STDOUT_FILENO,"\n",1);
      if(close(pfds[0]) == -1)
        errExit("close");
      _exit(EXIT_SUCCESS);
    default:
      //parent writes to pipe 
      if(close(pfds[0]) == -1)
        errExit("close - parent");

      if(write(pfds[1],argv[1],(long)strlen(argv[1])) != (long)strlen(argv[1]))
        fatal("parent-partial/failed write");

      if(close(pfds[1]) == -1)
        errExit("close");

      //wait child to finish
      wait(NULL);
      exit(EXIT_SUCCESS);
  }
}
