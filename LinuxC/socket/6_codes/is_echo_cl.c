#include "tlpi_hdr.h"
#include "inet_sockets.h"

#define BUF_SIZE 100

int main(int argc,char* argv[]){
  if(argc != 2 || strcmp(argv[1],"--help") == 0)
    usageErr("%s host\n",argv[0]);

  int sfd = inetConnect(argv[1],"echo",SOCK_STREAM);
  if(sfd == -1)
    errExit("inetConnect");
  
  ssize_t numRead;
  char buf[BUF_SIZE];
  switch(fork()){
    case -1:
      errExit("fork");
    case 0:
      //child read server's response,echo on stdout
      for(;;){
        numRead = read(sfd,buf,BUF_SIZE);
        if(numRead <= 0)
          break;
        printf("%.*s",(int)numRead,buf);
      }
      exit(EXIT_SUCCESS);
    default:
      //Parent writes contents of stdin to socket
      for(;;){
        numRead=read(STDIN_FILENO,buf,BUF_SIZE);
        if(numRead <= 0)
          break;
        if(write(sfd,buf,numRead)!= numRead)
          fatal("write() failed");
      }
      //close writing channle.So the server sees EOF
      if(shutdown(sfd,SHUT_WR) == -1)
        errExit("shutdown");
      exit(EXIT_SUCCESS);
  }
}
