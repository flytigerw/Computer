#include "svmsg_file.h"

static int clientId;


//Exit handler.Ensure the message queue is deleted
static void removeQueue(void){
    if(msgctl(clientId,IPC_RMID,NULL) == -1)
      errExit("msgctl11");
}


int main(int argc,char* argv[]){
  if(argc != 2)
    usageErr("%s pathname\n",argv[0]);
  
  struct requestMsg req;
  if(strlen(argv[1]) > sizeof(req.pathname) -1)
    cmdLineErr("pathname too long(max:%ld bytes\n)",(long)sizeof(req.pathname)-1);
  
  //Get server's queue-id 
  int serverId = msgget(SERVER_KEY,S_IWUSR);
  if(serverId == -1)
    errExit("msgget - server message queue");
  
  //create queue for response 
  int clientId = msgget(IPC_PRIVATE,S_IRUSR | S_IWUSR | S_IWGRP);
  if(clientId == -1)
    errExit("msgget - client message queue");

  if(atexit(removeQueue) != 0)
    errExit("atexit");

  //send message asking for file named in argv[1]
  req.mtype = 1;
  req.clientId = clientId;
  strncpy(req.pathname,argv[1],sizeof(req.pathname)-1);
  req.pathname[sizeof(req.pathname)-1]='\0';

  if(msgsnd(serverId,&req,REQ_MSG_SIZE,0) == -1)
    errExit("msgsnd");
  
  //get first response,which may be failure notification
  struct responseMsg resp;
  ssize_t msgLen = msgrcv(clientId,&resp,RESP_MSG_SIZE,0,0);
  if(msgLen == -1)
    errExit("msgrcv");

  if(resp.mtype == RESP_MT_FAILURE){
   printf("%s\n",resp.data);
   if(msgctl(clientId,IPC_RMID,NULL) == -1)
     errExit("msgctl");
   exit(EXIT_FAILURE);
  }
  
  //File was opened successfully by server;process messages containing file data 
  ssize_t totBytes = msgLen;
  int i;
  for( i=1;resp.mtype == RESP_MT_DATA;i++){
    msgLen = msgrcv(clientId,&resp,RESP_MSG_SIZE,0,0);
    if(msgLen == -1)
      errExit("msgrcv");
    totBytes += msgLen;
  }
  printf("Received %ld bytes (%d messages)\n",(long)totBytes,i);
  exit(EXIT_SUCCESS);
}
