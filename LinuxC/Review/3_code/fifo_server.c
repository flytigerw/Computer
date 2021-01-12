#include <signal.h>
#include "fifo_CS.h"



int main(int argc,char* argv[]){

  //create server-fifo 
  umask(0);                     //We can get the permissions we want 
  // 权限为:rw-w----
  if(mkfifo(SERVER_FIFO,S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
    errExit("mkfifo%s",SERVER_FIFO);
  //open it for reading 
  int s_fd = open(SERVER_FIFO,O_RDONLY);
  if(s_fd == -1)
    errExit("open server fifo");
  //open it for writing -----> The server won't see EOF when no clients have open fd for write
  if(open(SERVER_FIFO,O_WRONLY) == -1)
    errExit("open server fifo for writing");

  //Ignore SIGPIPE signal ---->  If the server attempts to writes into a client FIFO that doesn't have a reader,then it won't receive SIGPIPE signal which kills the process by default.Instead,it receives EPIPE errno 
  if(signal(SIGPIPE,SIG_IGN) == SIG_ERR)
    errExit("signal");

  //Handle clients iteratively
  //receive client requests,take out the pid,build the client fifo name,open it, send response 
  char client_fifo[CLIENT_FIFO_NAME_LEN];
  struct response resp;
  struct request req;             // buffer
  int c_fd;
  int msg_seq = 0;
  for(;;){
    if(read(s_fd,&req,sizeof(struct request)) != sizeof(struct request)){
      fprintf(stderr,"Error reading request! discarding it\n");
      continue;
    }
    //build client fifo name
    snprintf(client_fifo,CLIENT_FIFO_NAME_LEN,CLIENT_FIFO_TEMPLATE,(long)req.pid);
    
    //open it for writing 
    c_fd = open(client_fifo,O_WRONLY);
    if(c_fd == -1){
      errMsg("open %s",client_fifo);
      continue;
    }
    
    resp.msg_seq = msg_seq;
    //send response 
    if(write(c_fd,&resp,sizeof(struct response)) != sizeof(struct response))
      fprintf(stderr,"Error writing to %s\n",client_fifo);
    
    //close client fd 
    if(close(c_fd) == -1)
      errMsg("close");

    msg_seq += req.msg_len;
  }
}
