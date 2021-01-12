
#include "unix_datagram_common.h"
#include <ctype.h>

int main(int argc,char* argv[]){
  //create the socket
  int fd = socket(AF_UNIX,SOCK_DGRAM,0);
  if(fd == -1)
    errExit("socket");


  //construct the server address  and bind it 
  if(remove(PATHNAME) == -1 && errno != ENOENT)
    errExit("remove %s",PATHNAME);

  struct sockaddr_un s_addr,c_addr;
  memset(&s_addr,0,sizeof(struct sockaddr_un));
  s_addr.sun_family = AF_UNIX;
  strncpy(s_addr.sun_path,PATHNAME,sizeof(s_addr.sun_path)-1);
  
  if(bind(fd,(struct sockaddr*)&s_addr,sizeof(struct sockaddr_un)) == -1)
    errExit("bind");

  ssize_t num_read;
  socklen_t len;
  char buf[BUFSIZE];
  for(;;){
    len = sizeof(struct sockaddr_un);
    num_read = recvfrom(fd,buf,BUFSIZE,0,(struct sockaddr*)&c_addr,&len);
    if(num_read == -1)
      errExit("recvfrom");
    printf("Server received %ld butes from %s\n",(long)num_read,c_addr.sun_path);

    for(int i=0;i<num_read;i++)
      buf[i] = toupper((unsigned char)buf[i]);
    if(sendto(fd,buf,num_read,0,(struct sockaddr*)&c_addr,len) != num_read)
      fatal("sendto");
  }
}
