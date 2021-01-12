#include "fifo_seqnum.h"

static char clientInfo[CLIENT_FIFO_NAME_LEN];

//Invoked on exit to delete client FIFO
static void removeFifo(void){
    unlink(clientInfo);
}

int main(int argc,char* argv[]){
    if(argc>1 && strcmp(argv[1],"--help") == 0)
      usageErr("%s [seq-len]\n",argv[0]);

    umask(0);                 //So we get the permissions we want
    //create client FIFO to send requst
    snprintf(clientInfo,CLIENT_FIFO_NAME_LEN,CLIENT_FIFO_TEMPLATE,(long)getpid());
    if(mkfifo(clientInfo,S_IRUSR|S_IWUSR|S_IWGRP) == -1 && errno != EEXIST) 
      errExit("mkfifo %s",clientInfo);

    if(atexit(removeFifo) != 0)
      errExit("atexit");

    struct request req;
    struct response resp;
    req.pid = getpid();
    req.seqLen = (argc > 1) ? getInt(argv[1],GN_GT_0,"seqLen") : 1;
    int serverFd = open(SERVER_FIFO,O_WRONLY);
    if(serverFd == -1)
      errExit("open%s",SERVER_FIFO);

    if(write(serverFd,&req,sizeof(struct request)) != sizeof(struct request))
      fatal("can't write to server");

    //Open FIFO for reading response 
    int clientFd = open(clientInfo,O_RDONLY);
    if(clientFd == -1)
      errExit("open %s",clientInfo);

    if(read(clientFd,&resp,sizeof(struct response)) != sizeof(struct response))
      fatal("can't read response from server");

    printf("%d\n",resp.seqNum);
    exit(EXIT_SUCCESS);
}
