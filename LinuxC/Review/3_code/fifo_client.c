

#include "fifo_CS.h"


static char client_fifo[CLIENT_FIFO_NAME_LEN];


//unlink fifo at exit
static void remove_fifo(void){
      unlink(client_fifo);
}

int main(int argc,char* argv[]){

  //create client_fifo fifo 
  snprintf(client_fifo,CLIENT_FIFO_NAME_LEN,CLIENT_FIFO_TEMPLATE,(long)getpid());
  umask(0);
  if(mkfifo(client_fifo,S_IRUSR | S_IWUSR | S_IWGRP) ==-1 && errno != EEXIST)
    errExit("mkfifo");
  
  if(atexit(remove_fifo) == -1)
    errExit("atexit");


  //construct request 
  struct request req;
  req.pid = getpid();
  //use the second argument as the msg_len.By default,it's 1
  int x = (argc > 1) ? getInt(argv[1],GN_GT_0,"msg_len") : 1;
  req.msg_len = x;

  //open server fifo 
  int s_fd = open(SERVER_FIFO,O_WRONLY);
  if(s_fd == -1)
    errExit("open server fifo");

  //send struct requests 
  if(write(s_fd,&req,sizeof(struct request)) != sizeof(struct request))
    errExit("write");

  //open client_fifo for reading
  int c_fd = open(client_fifo,O_RDONLY);
  if(c_fd == -1)
    errExit("open client_fifo");

  //reveive struct response from client fifo
  struct response resp;
  if(read(c_fd,&resp,sizeof(resp)) != sizeof(resp))
    errExit("read");

  //output the struct response 
  printf("%d\n",resp.msg_seq);
  exit(EXIT_SUCCESS);
}
