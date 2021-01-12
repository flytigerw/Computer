#include  "us_xfr.h"

#define BACKLOG 5

//iterative server: handle a client at a time before proceeding next client
int main(int argc, char* argv[]){

  //create stream socket 
  int sfd = socket(AF_UNIX,SOCK_STREAM,0);
  if(sfd == -1)
    errExit("socket");

  //construct server socket  address 
  struct sockaddr_un serverAddr;
  memset(&serverAddr,0,sizeof(struct sockaddr_un));
  serverAddr.sun_family = AF_UNIX;
  if(remove(SV_SOCK_PATH) == -1 && errno != ENOENT)
    errExit("remove-%s",SV_SOCK_PATH);
  strncpy(serverAddr.sun_path,SV_SOCK_PATH,sizeof(serverAddr.sun_path)-1);

  //bind to well-known address 
  if(bind(sfd,(struct sockaddr*)&serverAddr,sizeof(struct sockaddr_un)) == -1)
    errExit("bind");
  //listen to wait for the connection req 
  if(listen(sfd,BACKLOG) == -1)
    errExit("listen");

  //handle client connection req iteratively 
  int cfd;
  ssize_t numRead;
  char buf[BUFSIZE];
  for(;;){
    //accept a connection and create a new socket and return its fd 
    if((cfd = accept(sfd,NULL,NULL)) == -1)
      errExit("listen");
    //Transfer data from connected socket to stdout until EOF
    while((numRead= read(cfd,buf,BUFSIZE)) >0 ){
      if(write(STDOUT_FILENO,buf,numRead) != numRead)
        fatal("partial/failed write");
    }
    if(numRead == -1)
      errExit("read");
    if(close(sfd) == -1)
      errMsg("close");
  }
}
