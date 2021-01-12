#define  _BSD_SOURCE //to get defination os NI_MAXHOST and NI_MAXSERV

#include <netdb.h>
#include "is_seqnum.h"

#define ADDRSTRLEN (NI_MAXHOST+NI_MAXSERV+10)
#define BACKLOG 50

int main(int argc,char* argv[]){
  if(argc > 1 && strcmp(argv[1],"--help") == 0)
    usageErr("%s [init-seq-num]\n");
  
  uint32_t seqNum = (argc>1)? getInt(argv[1],0,"init-seq-num") : 0;
  //ignore SIGPIPE:prevents the sever from receivig the SIGPIPE signal if it tries to write to sokcet whose peer has been closed.Instead write() fails with the error EPIPE
  if(signal(SIGPIPE,SIG_IGN) == SIG_ERR)
    errExit("signal");

  //call addrinfo() to obtain a list of address that we can bind to
  struct addrinfo hints;
  struct addrinfo* result,*tmp;
  memset(&hints,0,sizeof(struct addrinfo));
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
  //AI_PASSIVE:the resulting socket will be bound to the wildcard address
  if(getaddrinfo(NULL,PORT_NUM,&hints,&result) != 0)
    errExit("getaddrinfo");
  
  //walk through the returned list until we find an address structure that can be successfully used to create and bind a socket
  int optval = 1;
  int sfd;
  for(tmp = result; tmp != NULL; tmp=tmp->ai_next){
     sfd=socket(tmp->ai_family,tmp->ai_socktype,tmp->ai_protocol);
     if(sfd == -1)
       continue;
     if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval)) == -1)
       errExit("setsockopt");
    if(bind(sfd,tmp->ai_addr,tmp->ai_addrlen) == 0)
      break;        //success
    //bind() faild:close this socket and try another address 
    close(sfd);
  }
  if(tmp == NULL)
    fatal("couldn't bind socket to any address");

  if(listen(sfd,BACKLOG) == -1)
    errExit("listen");

  freeaddrinfo(result);

  //hand clients iteratively
  socklen_t addrlen;
  struct sockaddr_storage claddr;
  int cfd;
  char host[NI_MAXHOST];
  char service[NI_MAXSERV];
  char addrStr[ADDRSTRLEN];
  char reqLenStr[INT_LEN];
  char seqNumStr[INT_LEN];
  int reqLen;
  for(;;){
    //accept a client connection,obtain the client's address
    addrlen = sizeof(struct sockaddr_storage);
    cfd = accept(sfd,(struct sockaddr*) &claddr,&addrlen);
    if(cfd == -1){
      errMsg("accept");
      continue;
    }
    if(getnameinfo((struct sockaddr*)&claddr,addrlen,host,NI_MAXHOST,service,NI_MAXSERV,0) == 0)
      snprintf(addrStr,ADDRSTRLEN,"(%s,%s)",host,service);
    else  
      snprintf(addrStr,ADDRSTRLEN,"(?UNKNOWN?)");
    //read client requesr,send sequence number back 
    if(readLine(cfd,reqLenStr,INT_LEN) <= 0){
      close(cfd);
      continue;
    }
    reqLen = atoi(reqLenStr);
    if(reqLen <= 0 ){
      close(cfd);
      continue;
    }
    snprintf(seqNumStr,INT_LEN,"%d\n",seqNum);
    if(write(cfd,&seqNumStr,strlen(seqNumStr)) != (ssize_t)strlen(seqNumStr))
      fprintf(stderr,"Error on write");
    seqNum += reqLen;  //update sequence number 
    if(close(cfd) == -1)
      errMsg("close");
  }
}
