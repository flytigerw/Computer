

#define _GNU_SOURCE
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sched.h>
#include "tlpi_hdr.h"

#ifndef CHILD_SIG
#define CHILD_SIG SIGUSR1     //signal to be generated on termination of cloned child 
#endif 

//startup func for cloned child 
static int 
childFunc(void* arg){
  if(close(*(int*)arg) == -1)   //arg 解释为fd
    errExit("close");
  return 0;
}

int main(int argc,char* argv[]){
    //open /dev/null and let child close 
    int fd = open("/dev/null",O_RDWR);
    if(fd == -1)
      errExit("open");

    //if argc>1,child shares file descriptor table with parent
    int flags = argc>1 ? CLONE_FILES:0;
    //allocate stack for child 
    const int STACK_SIZE = 65536;
    char* stack = (char*)malloc(STACK_SIZE);
    if(stack == NULL)
      errExit("mallc");
    char* stackTop = stack+STACK_SIZE;
    //ignore CHILD_SIG in case it is a signal whose default is to terminate the process
    //dont ignore SIGCHLD,since that would prevent the creation of a zombie 
    if(CHILD_SIG !=0 && CHILD_SIG != SIGCHLD)
      if(signal(CHILD_SIG,SIG_IGN) == SIG_ERR)
        errExit("signal");

    //created clone child 
    if(clone(childFunc,stackTop,flags|CHILD_SIG,(void*)&fd) == -1)
      errExit("clone");

    //parent wait for child 
    if(waitpid(-1,NULL,(CHILD_SIG != SIGCHLD)? __WCLONE:0) == -1)
      errExit("waitpid");
    printf("child has terminated\n");
    
    //test if close of fd in child affect parent 
    int s = write(fd,"x",1);
    if(s == -1 && errno==EBADF)
      printf("fd %d has been closed\n",fd);
    else if(s == -1)
      printf("write on fd %d failed.Unexpectedly (%s)\n",fd,strerror(errno));
    else 
      printf("write on fd %d succeeded\n",fd);
    
    exit(EXIT_SUCCESS);
}
