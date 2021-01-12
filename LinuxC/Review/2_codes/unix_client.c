
#include "unix_common.h"

int main(int argc,char* argv[]){
  //create the  socket 
  int fd = socket(AF_UNIX,SOCK_STREAM,0);
  if(fd == -1)
    errExit("socket");
  
  //we needn't bind it 
  //Just construct the server address and connect to it 
  struct sockaddr_un addr;
  memset(&addr,0,sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path,PATHNAME,sizeof(addr.sun_path)-1);
  if(connect(fd,(struct sockaddr*)&addr,sizeof(struct sockaddr_un)) == -1)
    errExit("connect");
  //read data from stdin and write it to socket 
  ssize_t num_read;
  char buf[BUFSIZE];
  while((num_read = read(STDIN_FILENO,buf,BUFSIZE)) != 0)
    if(write(fd,buf,num_read) != num_read)
      errExit("write");
  if(num_read == -1)
    errExit("read");
  exit(EXIT_SUCCESS);
}
