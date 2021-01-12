#include "inet_sockets.h"
#include "tlpi_hdr.h"

int main(int argc,char* argv[]){

  if(argc != 2 || strcmp(argv[1],"--help") == 0)
    usageErr("%s service\n",argv[0]);
  socklen_t len;
  int listenFd = inetListen(argv[1],5,&len);
  if(listenFd == -1)
    errExit("inetListen");

  int connFd = inetConnect(NULL,argv[1],SOCK_STREAM);
  if(connFd == -1)
    errExit("inetConnect");
  int acceptFd = accept(listenFd,NULL,NULL);
  if(acceptFd == -1)
    errExit("accept");
  char addrStr[IS_ADDR_STR_LEN];
  void* addr;
  addr = malloc(len);
  if(addr == NULL)
    errExit("malloc");

  if(getsockname(connFd,(struct sockaddr*)&addr,&len) == -1)
    errExit("getsockname");
  printf("getsockname(connFd): %s\n",inetAddressStr((struct sockaddr*)&addr,len,addrStr,IS_ADDR_STR_LEN));

  if(getsockname(acceptFd,(struct sockaddr*)&addr,&len) == -1)
    errExit("getsockname");
  printf("getsockname(acceptFd): %s\n",inetAddressStr((struct sockaddr*)&addr,len,addrStr,IS_ADDR_STR_LEN));

  if(getpeername(connFd,(struct sockaddr*)&addr,&len) == -1)
    errExit("getpeername");
  printf("getpeername(connFd):  %s\n",inetAddressStr((struct sockaddr*)&addr,len,addrStr,IS_ADDR_STR_LEN));

  if(getpeername(acceptFd,(struct sockaddr*)&addr,&len) == -1)
    errExit("getpeername");
  printf("getpeername(acceptFd):  %s\n",inetAddressStr((struct sockaddr*)&addr,len,addrStr,IS_ADDR_STR_LEN));

  sleep(30);       //spare some time to run netstat 
  exit(EXIT_SUCCESS);
}
