#include <signal.h>
#include "fifo_seqnum.h"

int main(int argc,char* argv[]){
    umask(0);                   //so we get the permissions we want

    //create server FIFO for receiving response
    if(mkfifo(SERVER_FIFO,S_IRUSR|S_IWUSR|S_IWGRP) == -1 && errno != EEXIST)
      errExit("mkfifo %s",SERVER_FIFO);
    //Open it
    int serverFd = open(SERVER_FIFO,O_RDONLY);
    if(serverFd == -1)
      errExit("open %s",SERVER_FIFO);

    //Open an extra write fd,so that we never see EOF 
    int dummyFd = open(SERVER_FIFO,O_WRONLY);
    if(dummyFd == -1)
      errExit("open %s",SERVER_FIFO);
    
    //Ignore SIGPIPE so that if the sever attempts to write to client FIFO that does'nt have a reader,then rather than being sent SIGPIPE which kills a process by default,it receives an EPIPE error form write()
    if(signal(SIGPIPE,SIG_IGN) == SIG_ERR)
      errExit("signal");
    
    struct request req;
    struct response resp;
    int clientFd;
    int seqNum = 0;
    char clientInfo[CLIENT_FIFO_NAME_LEN];
    for(;;){  //read requests and send response
      if(read(serverFd,&req,sizeof(struct request)) != sizeof(struct request)){
            fprintf(stderr,"Error reading request;discarding\n");
            continue;
      }
      //Open client FIFO (previously created by client)
      snprintf(clientInfo,CLIENT_FIFO_NAME_LEN,CLIENT_FIFO_TEMPLATE,(long)req.pid);  
      clientFd = open(clientInfo,O_WRONLY);
      if(clientFd == -1){  //Open failed,give up on client 
        errMsg("open %s",clientInfo);
        continue;
      }
     //send response and close FIFO 
     resp.seqNum = seqNum;
     if(write(clientFd,&resp,sizeof(struct response)) != sizeof(struct response))
       fprintf(stderr,"Error writing to FIFO %s\n",clientInfo);
     if(close(clientFd) == -1)
       errMsg("close");
     seqNum += req.seqLen;
    }
}
