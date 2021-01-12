#include <syslog.h>
#include "id_echo.h"
#include "become_daemon.h"

int main(int argc,char* argv[]){

//becomeDaemon function turn server into a daemon
  if(becomeDaemon(0) == -1)
    errExit("becomeDaemon");
  
  int sfd;
  socklen_t addrLen;
  //create UDP server socket of given type bound to wildcard IP address
  sfd = inetBind(SERVICE,SOCK_DGRAM,&addrLen);
  if(sfd == -1){
    syslog(LOG_ERR,"Could't create server socket(%s)",strerror(errno));
    exit(EXIT_FAILURE);
  }

  //receive datagrams and return copies to senders
  socklen_t len;
  struct sockaddr_storage claddr; 
  char buf[BUF_SIZE];
  char addrStr[IS_ADDR_STR_LEN];
  ssize_t numRead;
  for(;;){
     len = sizeof(struct sockaddr_storage);
     numRead = recvfrom(sfd,buf,BUF_SIZE,0,(struct sockaddr*)&claddr,&len);
     printf("recvfrom client:%s\n",buf);
     if(numRead == -1)
       errExit("recvfrom");
     if(sendto(sfd,buf,numRead,0,(struct sockaddr*)&claddr,len) != numRead){
       syslog(LOG_WARNING,"Error echoing response to %s(%s)",
           inetAddressStr((struct sockaddr*)&claddr,len,addrStr,IS_ADDR_STR_LEN),strerror(errno));
     }
  }
}

