#include "i6d_ucase.h"

int main(int argc,char* argv[]){

  //create server socket 
  int sfd = socket(AF_INET6,SOCK_DGRAM,0);
  if(sfd == -1)
    errExit("socket");
  //bind server socket to a addr 
  struct sockaddr_in6 svaddr;
  memset(&svaddr,0,sizeof(struct sockaddr_in6));
  svaddr.sin6_family = AF_INET6;
  svaddr.sin6_addr = in6addr_any;   //Wildcard address 
  svaddr.sin6_port =htons(PORT_NUM);
  if(bind(sfd,
        (struct sockaddr*)&svaddr,
        sizeof(struct sockaddr_in6)) == -1)
    errExit("bind");
  
  //receive messages,convert to uppercase,and return to client
  socklen_t len;
  char buf[BUF_SIZE];
  char claddrStr[INET6_ADDRSTRLEN];
  ssize_t numBytes;
  struct sockaddr_in6 claddr;
  for(;;){
     len = sizeof(struct sockaddr_in6);
     numBytes = recvfrom(sfd,buf,BUF_SIZE,0,
                        (struct sockaddr*)&claddr,&len); //the addr of clien is placed in claddr in a form of binary
     if(numBytes == -1)
       errExit("recvfom");
     
     //convert client address to readable format 
     if(inet_ntop(AF_INET6,&claddr.sin6_addr,claddrStr,INET6_ADDRSTRLEN) == NULL)
       printf("couldn't convert client address to string\n");
     else 
       printf("Server received %ldbytes from(%s,%u)\n",(long)numBytes,claddrStr,ntohs(claddr.sin6_port));

     //To uppercase 
     for(int i=0;i<numBytes;i++)
       buf[i]=toupper((unsigned char)buf[i]);

     //send back 
     if(sendto(sfd,buf,numBytes,0,
           (struct sockaddr*)&claddr,len) != numBytes)
       fatal("sendto");
  }
  
}
