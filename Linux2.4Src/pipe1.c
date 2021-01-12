



#include <stdio.h> 
#include <unistd.h>
#include <wait.h>




int main(){

  int B,C,fds[2];
  char* B_args[] = {"/bin/wc",NULL};
  char* C_args[] = {"/usr/bin/ls","-l",NULL};

  pipe(fds);

  switch(B = fork()){
    case 0 :
      //process B 
    
      //close read end 
      close(fds[0]);
      //rediret write end to STDOUT
      close(1);
      dup2(fds[1],1);
      close(fds[1]);

      execve("/usr/bin/wc",B_args,NULL);
      printf("pid %d: I am back. Sth is wrong!\n",getpid());
    default:
    //father process 
    //close write end 
    close(fds[0]);
  }

  switch(C = fork()){
    case 0 :
      //process C
      
      //rediret read end to STDIN
      close(0);
      dup2(fds[0],0);
      close(fds[0]);

      execve("/bin/ls",C_args,NULL);
      printf("pid %d: I am back. Sth is wrong!\n",getpid());
    default:
    //father process 
    //close read end 
    close(fds[0]);
  }

  wait4(C,NULL,0,NULL);
  printf("done");
  return  0;
}
