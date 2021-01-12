
#include <sys/time.h>
#include <sys/select.h>
#include <string.h>
#include "tlpi_hdr.h"

static void usageError(const char* progname){
  fprintf(stderr,"Usage :%s {timeout|-}fd-num[rw]...\n",progname);
  fprintf(stderr," - means infinite timeout; \n");
  fprintf(stderr," r = monitor for read\n");
  fprintf(stderr," w = monitor for write\n");
  fprintf(stderr," e.g:%s - Orw 1w\n",progname);
  exit(EXIT_FAILURE);
}

int main(int argc,char* argv[]){
  if(argc < 2)
    usageError(argv[0]);
  //timeout is specified in argvp[1]
  struct timeval* pto;
  struct timeval timeout;
  if(strcmp(argv[1],"-") == 0){
      pto = NULL;             //Infinite timeout
  }else{
    pto = &timeout;
    timeout.tv_sec = getLong(argv[1],0,"timeout");
    timeout.tv_usec = 0; 
  }

  //remaining arguments are used to build fdsets
  int nfds = 0;
  fd_set readfds,writefds;
  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  int numRead;
  char buf[10];
  int fd;
  for(int i=2; i<argc; i++){
      numRead = sscanf(argv[i],"%d%2[rw]",&fd,buf);
      if(numRead != 2)
        usageError(argv[0]);
      if(fd >= FD_SETSIZE)
        cmdLineErr("fd exceeds limit (%d)\n",FD_SETSIZE);
      if(fd >= nfds)
        nfds = fd+1;
      if(strchr(buf,'r') != NULL)
        FD_SET(fd,&readfds);
      if(strchr(buf,'w') != NULL)
        FD_SET(fd,&writefds);
  }

  //we have build all the arguments.Now call select()
  int ready = select(nfds,&readfds,&writefds,NULL,pto);
  if(ready == -1)
    errExit("select");

  //Display results of select 
  printf("ready = %d\n",ready);
  for(fd = 0; fd < nfds; fd++)
    printf("%d: %s%s\n",fd,FD_ISSET(fd,&readfds) ? "r" : "",FD_ISSET(fd,&writefds) ? "w" : "");
  if(pto != NULL)
    printf("timeout after select():% ld.%03ld\n",(long)timeout.tv_sec,(long)timeout.tv_usec/10000);
  exit(EXIT_SUCCESS);
}
