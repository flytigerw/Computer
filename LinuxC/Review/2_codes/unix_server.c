
#include "unix_common.h"

#define BACKLOG 5

int main(int argc,char* argv[]){

  //create a socket
  int fd = socket(AF_UNIX,SOCK_STREAM,0);
  if(fd == -1)
    errExit("socket");
  //construct the socket address and bind it to the pathname
  //clear 
  if(remove(PATHNAME) == -1 && errno != ENOENT)
    errExit("remove %s",PATHNAME);
  struct sockaddr_un addr;
  memset(&addr,0,sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path,PATHNAME,sizeof(addr.sun_path)-1);
  if(bind(fd,(struct sockaddr*)&addr,sizeof(struct sockaddr)) == -1)
    errExit("bind");

  //listen
  if(listen(fd,BACKLOG) == -1)
    errExit("listen");

  //Handle client requests iteratively --> accept(),read() data and write() data to stdout
  char buf[BUFSIZE];
  int sfd;
  ssize_t num_read;
  for(;;){
  
    sfd = accept(fd,NULL,0);  //we are not intereste in the source address
    while((num_read = read(sfd,buf,BUFSIZE)) != 0)
      if(write(STDOUT_FILENO,buf,num_read) != num_read)
        errExit("write");
    //处理read()时的出错情况
    if(num_read == -1)
      errExit("read");
    //服务结束以后要归还资源
    if(close(sfd) == -1)
      errExit("close");
  }

  
}
