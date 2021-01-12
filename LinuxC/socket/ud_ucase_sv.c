//iterative server : handles one client at a time before proceeding to the next client

#include "ud_ucase.h"

int main(int argc,char* argv[]){
  //create server socket 
  int sfd = socket(AF_UNIX,SOCK_DGRAM,0);
  if(sfd == -1)
    errExit("socket");

  //construct a well-known pathname and bind server socket to it 
  if(remove(SV_SOCK_PAHT) == -1 && errno != ENOENT)          //we can't bind a socket to an existing file
    errExit("remove-%s",SV_SOCK_PAHT);

  struct sockaddr_un serverAddr, clientAddr;
  memset(&serverAddr,0,sizeof(struct sockaddr_un));
  strncpy(serverAddr.sun_path,SV_SOCK_PAHT,sizeof(serverAddr.sun_path)-1);
  serverAddr.sun_family = AF_UNIX;

  if(bind(sfd,(struct sockaddr*)&serverAddr,sizeof(struct sockaddr_un)) == -1)
    errExit("bind");

  //receive messages and convert to uppercase and return them to client 
  ssize_t numBytes;
  socklen_t len;
  char buf[BUF_SIZE];
  for(;;){
    len = sizeof(struct sockaddr_un);
    numBytes = recvfrom(sfd,buf,BUF_SIZE,0,(struct sockaddr*)&clientAddr,&len);
    if(numBytes == -1)
      errExit("recvfrom");
    printf("Server received %ld butes from %s\n",(long)numBytes,clientAddr.sun_path);
    
    //convert to uppercase
    for(int i=0; i<numBytes;i++)
      buf[i] = toupper((unsigned char)buf[i]);

    //return to client 
    if(sendto(sfd,buf,numBytes,0,(struct sockaddr*)&clientAddr,len) != numBytes)
      fatal("sendto");
  }
}
