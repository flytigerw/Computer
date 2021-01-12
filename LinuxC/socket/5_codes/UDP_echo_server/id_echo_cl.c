#include "id_echo.h"

int main(int argc,char* argv[]){
  if(argc<2 || strcmp(argv[1],"--help") == 0)
    usageErr("%s:host msg...\n",argv[0]);

  //construct server address from first cmd-line argument
  int sfd = inetConnect(argv[1],SERVICE,SOCK_DGRAM);
  if(sfd == -1)
    fatal("Could't connect to server socket");

  //Send remaining arguments to server 
  long len;
  ssize_t numRead;
  char buf[BUF_SIZE];
  for(int j=2;j<argc;j++){
    len = strlen(argv[j]);
    if(write(sfd,argv[j],len) != len)
      fatal("partial/failed write");
    printf("succeed write:%s\n",argv[j]);
    numRead = read(sfd,buf,BUF_SIZE);
    if(numRead == -1)
      errExit("read");
    printf("[%ld bytes] %.*s\n",(long)numRead,(int)numRead,buf);
  }
  exit(EXIT_SUCCESS);
}
