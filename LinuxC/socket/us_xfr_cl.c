#include "us_xfr.h"


int main(int argc,char* argv[]){
  
  //create client stream socket 
  int sfd = socket(AF_UNIX,SOCK_STREAM,0);
  if(sfd == -1)
    errExit("socket");
  
  //consruct serverAddr 
  struct sockaddr_un serverAddr;
  memset(&serverAddr,0,sizeof(struct sockaddr_un));
  serverAddr.sun_family = AF_UNIX;
  strncpy(serverAddr.sun_path,SV_SOCK_PATH,sizeof(serverAddr.sun_path)-1);
  
  //make the connetion
  if(connect(sfd, (struct sockaddr*)&serverAddr,sizeof(struct sockaddr_un)) == -1)
    errExit("connect");
  
  //read from stdin(重定向) and write through to peer socket 
  ssize_t numRead;
  char buf[BUFSIZE];
  while((numRead = read(STDIN_FILENO,buf,BUFSIZE)) >0 )
    if(write(sfd,buf,numRead) != numRead)
      fatal("partial/failed write");

  if(numRead == -1)
    errExit("read");

  exit(EXIT_SUCCESS);

}
