

#include <sys/wait.h>
#include "tlpi_hdr.h"

int main(int argc,char* argv[]){

  //create pipes 
  int pfds[2];
  if(pipe(pfds) == -1)
    errExit("pipe");
  
  //create child1 to exec ls later
  switch(fork()){
    case -1:
      errExit("fork1");
    case 0:
      //redirect stdout 
      if(pfds[1] != STDOUT_FILENO){
        dup2(pfds[1],STDOUT_FILENO);
        close(pfds[1]);
      }
      //exec ls 
      execlp("ls","ls",(char*)NULL);
      errExit("execlp ls");
    default:
      break;
  }

  //create child2 to exec wc
  switch(fork()){
    case -1:
      errExit("fork2");
    case 0:
      //redirect stdin
      if(pfds[0] != STDIN_FILENO){
        dup2(pfds[0],STDIN_FILENO);
        close(pfds[0]);
      }
      //exec ls 
      execlp("wc","wc","-l",(char*)NULL);
      errExit("execlp wc");
    default:
      break;
  }
  //close unsed fds 
  if(close(pfds[0]) == -1)
    errExit("close1");
  if(close(pfds[1]) == -1)
    errExit("close2");
  //wait for child to exit 
  if(wait(NULL) == -1)
    errExit("child1 error");
  if(wait(NULL) == -1)
    errExit("child2 error");
  exit(EXIT_SUCCESS);
}
