#include "svmsg_file.h" 


//SIGCHILD handler:msgrcv() call may block in the parent server process and consequently be interrupted by SIGCHLD handler. A loop is used to restart the call if it fails with EINTR error
static void grimHandler(int sig){
    int savedErrno = errno;
    while(waitpid(-1,NULL,WNOHANG) > 0)
      continue;
    errno = savedErrno;
}

//Executed in child process to server a single client
static void serveRequest(const struct requestMsg* req){
    int fd = open(req->pathname,O_RDONLY);
    struct responseMsg resp;
    if(fd == -1){      //Opend failed,send error text 
      resp.mtype = RESP_MT_FAILURE; 
      snprintf(resp.data,sizeof(resp.data),"%s","couldn't open");
      msgsnd(req->clientId,&resp,strlen(resp.data)+1,0);
      exit(EXIT_FAILURE);
    }
    //Transmit file contents in messages with type RESP_MT_DATA 
    resp.mtype = RESP_MT_DATA;
    ssize_t numRead;
    while((numRead = read(fd,resp.data,RESP_MSG_SIZE)) > 0)
        if(msgsnd(req->clientId,&resp,numRead,0) == -1)
          break;
    
    //Send a message of type RESP_MT_END to signify EOF
    resp.mtype = RESP_MT_END;
    msgsnd(req->clientId,&resp,0,0);            //zero-length text
}

int main(int argc,char* argv[]){
  //create server message queue 
  int serverId = msgget(SERVER_KEY,IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IWGRP);
  if(serverId == -1)
    errExit("msgget");
  
  //Establish SIGCHLD handler to reap terminated children 
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_handler = grimHandler;
  sa.sa_flags = SA_RESTART;
  if(sigaction(SIGCHLD,&sa,NULL) == -1)
    errExit("sigaction");

  //Read requests, handle each in child process 
  ssize_t msgLen;
  struct requestMsg req;
  pid_t pid;
  for(;;){
    msgLen = msgrcv(serverId,&req,REQ_MSG_SIZE,0,0);
    if(msgLen == -1){
      if(errno == EINTR)    //interrupted by SIGCHLD handler .. restart it 
        continue;
      errMsg("msgrcv");
      break;
    }
    pid = fork();
    if(pid == -1){
      errMsg("fork");
      break;
    }
    if(pid == 0){
      serveRequest(&req);
      _exit(EXIT_SUCCESS);
    }
  }
  //if msgrcv() or fork() fails,remove server MQ and exit 
  if(msgctl(serverId,IPC_RMID,NULL) == -1)
    errExit("msgctl---");
  exit(EXIT_SUCCESS);
}
