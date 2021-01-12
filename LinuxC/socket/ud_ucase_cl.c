
#include "ud_ucase.h"


//the sencond argument is the message need to be sent
int main(int argc,char* argv[]){
   if(argc < 2)
     usageErr("%s:msg..\n",argv[0]);
  
   //create a socket 
   int sfd = socket(AF_UNIX,SOCK_DGRAM,0);
   if(sfd == -1)
     errExit("socket");

   //bind to unique pathname(based on PID) so that server can send messages back 
   struct sockaddr_un serverAddr,clientAddr;
   memset(&clientAddr,0,sizeof(struct sockaddr_un));
   clientAddr.sun_family = AF_UNIX;
   snprintf(clientAddr.sun_path,sizeof(clientAddr.sun_path),"/tmp/ud_ucase_cl.%ld",(long)getpid());
   if(bind(sfd,(struct sockaddr*)&clientAddr,sizeof(struct sockaddr_un)) == -1)
     errExit("bind");

   //consruct the address of server 
   memset(&serverAddr,0,sizeof(struct sockaddr_un));
   serverAddr.sun_family = AF_UNIX;
   strncpy(serverAddr.sun_path,SV_SOCK_PAHT,sizeof(serverAddr.sun_path)-1);
   
   //send message to server 
   ssize_t msgLen;
   ssize_t numBytes;
   char buf[BUF_SIZE];
   int i =1;
   for(;i<argc;i++){
     msgLen = strlen(argv[i]);
     if(sendto(sfd,argv[i],msgLen,0,
           (struct sockaddr*)&serverAddr,
           sizeof(struct sockaddr_un)) != msgLen)
      fatal("sendto");

     //blocking receive 
     numBytes = recvfrom(sfd,buf,BUF_SIZE,0,NULL,NULL);
     if(numBytes == -1)
       errExit("recvfrom");

     printf("Response %d: %.*s\n",i,(int)numBytes,buf);
   }
   
   //remove client socket pathname 
   remove(clientAddr.sun_path);
   exit(EXIT_SUCCESS);
}
