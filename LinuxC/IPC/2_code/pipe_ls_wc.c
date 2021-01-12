#include <sys/wait.h>
#include "tlpi_hdr.h"

int main(int argc,char* argv[]){
  int pfd[2];
  if(pipe(pfd) == -1)
    errExit("pipe");

  switch(fork()){
    case -1:
      errExit("fork");
    case 0:
      //First child:exec 'ls' to write to pipe 
      if(close(pfd[0]) == -1)
        errExit("close 1");
      //redirect stdout to the write end 
      if(pfd[1] != STDOUT_FILENO){
        if(dup2(pfd[1],STDOUT_FILENO) == -1)
          errExit("dup2 1");
        if(close(pfd[1] == -1))
          errExit("close 2");
      }
      execlp("ls","ls",(char*) NULL);
      errExit("execlp ls");
    default:
      break;        //fall through to create next child
  }
  
  switch(fork()){
    case -1:
      errExit("fork");
    case 0:
      //second child:exec 'wc' to read from pipe
      if(close(pfd[1]) == -1)
        errExit("close 3");
      //redirect stdout to the write end 
      if(pfd[0] != STDIN_FILENO){
        if(dup2(pfd[0],STDIN_FILENO) == -1)
          errExit("dup2 2");
        if(close(pfd[0] == -1))
          errExit("close 4");
      }
      execlp("wc","wc","-l",(char*) NULL);
      errExit("execlp wc");
    default:
      break;        //fall through 
  }
  //parent closes the ununsed fds for pipe and waits for children 
  if(close(pfd[0]) == -1)
    errExit("close 5");
  if(close(pfd[1]) == -1)
    errExit("close 6");
  if(wait(NULL) == -1)
    errExit("wait 1");
  if(wait(NULL) == -1)
    errExit("wait 2");
  exit(EXIT_SUCCESS);
}
