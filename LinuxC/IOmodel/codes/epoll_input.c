
#include <sys/epoll.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#define MAX_BUF 1000        //Maximum bytes fetched by a single read()
#define ARRAY_SIZE 5        //Maximum number of events to be returned from a single epoll_wait()

int main(int argc,char* argv[]){
  //参数为filenames
  if(argc <2 )
    usageErr("%s file...\n",argv[0]);

  //create epoll instance 
  int epfd = epoll_create(0);
  if(epfd == -1) 
    errExit("epoll_create");

  //open each file on cmd line and add it to the interest list
  int fd;
  struct epoll_event ev;
  for(int i=1;i<argc;i++){
    fd = open(argv[i],O_RDONLY);
    if(fd == -1)
      errExit("open");
    printf("Opened \"%s\" on fd %d\n",argv[i],fd);
    ev.events = EPOLLIN;         //Only interesed in input events 
    ev.data.fd = fd;
    if(epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev) == -1)
      errExit("epoll_ctl");
  }
  
  int openfds = argc= - 1;
  int n;
  struct epoll_event evlist[ARRAY_SIZE];
  while(openfds > 0){
    printf("About to epoll_wait()\n");
    //monitor these fds
    n = epoll_wait(epfd,evlist,ARRAY_SIZE,-1);

    //error check
    if(n == -1){
      if(errno == EINTR)
        continue;              //restart if interrupted by signal
      else 
        errExit("epoll_wait");
    }

    printf("Ready amount:%d\n",n);
    

    char buf[MAX_BUF];
    int s;
    //Handl IO ready fds 
    
    for(int i=0;i<n;i++){
      printf("fd = %d;events:%s%s%s\n",
              evlist[i].data.fd,
              evlist[i].events & EPOLLIN ? "EPOLLIN" : "",
              evlist[i].events & EPOLLHUP ? "EPOLLHUP" : "",
              evlist[i].events & EPOLLOUT ? "EPOLLOUT" : "");
      if(evlist[i].events & EPOLLIN){
        s = read(evlist[i].data.fd,buf,MAX_BUF);
        if(s == -1)
          errExit("read");
        printf("  read %d bytes: %.*s\n",s,s,buf);
      }else if(evlist[i].events & (EPOLLHUP | EPOLLERR)){ //EPOLLIN and EPOLLHUP may be both set when there are more than MAX_BUF bytes to read and we read more bytes after the next epoll_wait().We close fd only if the EPOLLIN is not set
        printf("    closing fd %d\n",evlist[i].data.fd);
        if(close(evlist[i].data.fd) == -1)
          errExit("close");
        openfds--;
      }
    }
  }

  printf("All fds closed\n");
  exit(EXIT_SUCCESS);
}
