#include <signal.h>
#include <syslog.h>
#include <sys/wait.h>
#include "become_daemon.h"
#include "inet_sockets.h"
#include "tlpi_hdr.h"

#define SERVICE "echo"
#define BUF_SIZE 4096

//SIGCHLD handler to reap died child process 
static void grimReaper(int sig){
  int savedErrno = errno;   //save errno in case changed 
  while(waitpid(-1,NULL,WNOHANG) > 0)
    continue;
  errno  = savedErrno;
}

//handle a client request:copy socket input back to socket
static void handleRequest(int cfd){
  char buf[BUF_SIZE];
  ssize_t numRead;
  while((numRead=read(cfd,buf,BUF_SIZE)) > 0){
    if(write(cfd,buf,numRead) != numRead){
      syslog(LOG_ERR,"wite() failed:%s",strerror(errno));
      exit(EXIT_FAILURE);
    }
  }
  
  if(numRead == -1){
    syslog(LOG_ERR,"error from read():%s",strerror(errno));
    exit(EXIT_FAILURE);
  }
}

int main(int argc,char* argv[]){
  if(becomeDaemon(0) == -1)
    errExit("becomeDaemon");
  //install handler for SIGCHLD 
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = grimReaper;
  if(sigaction(SIGCHLD,&sa,NULL) == -1){
    syslog(LOG_ERR,"Error from sigaction():%s",strerror(errno));
    exit(EXIT_FAILURE);
  }
  int lfd = inetListen(SERVICE,10,NULL);
  if(lfd == -1){
    syslog(LOG_ERR,"could not create server socket(%s)",strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  int cfd;
  for(;;){
    //wait and then accept the connection
    cfd = accept(lfd,NULL,NULL);
    if(cfd == -1){
      syslog(LOG_ERR,"Failure in accept():%s",strerror(errno));
      exit(EXIT_FAILURE);
    }
    //handle each client request in a new child process 
    switch(fork()){
      case -1:
        syslog(LOG_ERR,"can't create child(%s)",strerror(errno));
        close(cfd);                         //Give up on this client and try next client
        break;
      case 0:
        close(lfd);         //Unneeded copy of listening socket 
        handleRequest(cfd);
        _exit(EXIT_SUCCESS);
    default:
        close(cfd);   //Unneeded copy of connected socket
        break;
    }
  }
}
