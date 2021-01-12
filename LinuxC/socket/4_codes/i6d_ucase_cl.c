
#include "i6d_ucase.h"

int main(int argc,char* argv[]){
  if(argc<3)
    usageErr("%s:host-address msg...\n",argv[0]);

  int sfd = socket(AF_INET6,SOCK_DGRAM,0);
  if(sfd == -1)
    errExit("socket");

  //form server socket address 
  struct sockaddr_in6 svaddr;
  memset(&svaddr,0,sizeof(struct sockaddr_in6));
  svaddr.sin6_family = AF_INET6;
  svaddr.sin6_port = htons(PORT_NUM);
  //In binary form 
  if(inet_pton(AF_INET6,argv[1],&svaddr.sin6_addr) <= 0)
    fatal("inet_pton faiked for address '%s'",argv[1]);
  //send messages to server; echo response on stdout 
  long msgLen;
  ssize_t numBytes;
  char resp[BUF_SIZE];
  for(int i=2;i<argc;i++){
    msgLen = strlen(argv[i]);
    if(sendto(sfd,argv[i],msgLen,0,(struct sockaddr*)&svaddr,sizeof(struct sockaddr_in6)) != msgLen)
      fatal("sendto");
    
    numBytes = recvfrom(sfd,resp,BUF_SIZE,0,NULL,NULL);
    if(numBytes == -1)
      errExit("recvfrom");
    
    printf("Response %d: %.*s\n",i-1,(int)numBytes,resp);
  }
  exit(EXIT_SUCCESS);
}

