#include <netdb.h>
#include "is_seqnum.h"

int main(int argc,char* argv[]){
  if(argc < 2 || strcmp(argv[1],"--help")==0)
    usageErr("%s server-host [sequence-len]\n",argv[0]);

  //call getaddrinfo() to obtain a list of addresses that we can try connecting to 
  struct addrinfo *result,*tmp;
  struct addrinfo hints;
  memset(&hints,0,sizeof(struct addrinfo));
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_NUMERICSERV;
  if(getaddrinfo(argv[1],PORT_NUM,&hints,&result) != 0)
    errExit("getaddrinfo");

  //walk through the returned list 
  int cfd;
  for(tmp=result; tmp != NULL; tmp=tmp->ai_next){
    cfd= socket(tmp->ai_family,tmp->ai_socktype,tmp->ai_protocol);
    if(cfd == -1)
      continue;
    if(connect(cfd,tmp->ai_addr,tmp->ai_addrlen) != -1)
      break;
    //connect failed:close this socket and try next address 
    close(cfd);
  }
  if(tmp == NULL)
    fatal("couldn't connect socket to any address");
  freeaddrinfo(result);
  //send requested sequence length,with terminating newline
  char seqNumStr[INT_LEN];   //start of granted sequence 
  const char* reqLenStr;           //requested lengtn of sequence
  reqLenStr = (argc>2) ? argv[2] :"1";
  if(write(cfd,reqLenStr,strlen(reqLenStr)) != (ssize_t)strlen(reqLenStr))
    fatal("Partial/failed write(reqLenStr)");
  if(write(cfd,"\n",1) != 1)
    fatal("Partial/failed write(newline)");

  //read and display sequence number returned by server 
  int numRead = readLine(cfd,seqNumStr,INT_LEN);
  if(numRead  == -1)
    errExit("readLine");
  if(numRead  == 0)
    fatal("Unexpected EOF from server");

  printf("Sequence number:%s",seqNumStr);
  
  exit(EXIT_SUCCESS);
}
