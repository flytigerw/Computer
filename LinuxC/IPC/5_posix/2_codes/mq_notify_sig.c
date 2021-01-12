#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#define NOTIFY_SIG SIGUSR1

//More can be seen in pdf 1081
//
static void handler(int sig){
  //just interrupt sigsuspend
}

int main(int argc,char* argv[]){
    if(argc != 2 || strcmp(argv[1],"--help")==0)
    usageErr("%s mq-name\n",argv[0]);

    mqd_t mqd = mq_open(argv[1],O_RDONLY | O_NONBLOCK);
    if(mqd == (mqd_t)-1)
      errExit("mq_open");
    struct mq_attr attr;
    if(mq_getattr(mqd,&attr) == -1)
      errExit("mq_getattr");
    
    void* buffer =malloc(attr.mq_msgsize);
    if(buffer == NULL)
      errExit("malloc");

    //block SIGUSR1 
    sigset_t blockMask,emptyMask;
    sigemptyset(&blockMask);
    sigaddset(&blockMask,NOTIFY_SIG);
    if(sigprocmask(SIG_BLOCK,&blockMask,NULL) == -1)
      errExit("sigprocmask");

    //install handler for SIGUSR1 
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler=handler;
    if(sigaction(NOTIFY_SIG,&sa,NULL) == -1)
      errExit("sigaction");
    
    //register
    struct sigevent sev;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = NOTIFY_SIG;
    if(mq_notify(mqd,&sev) == -1)
      errExit("mq_notify");
    
    ssize_t numRead;
    for(;;){
      //Unblock the signal and wait for notification siganl
      sigsuspend(&emptyMask);

      //notification has arrived and the process is deregistered ---> register again
      if(mq_notify(mqd,&sev) == -1)
        errExit("mq_notify");
      //Read as many as possible
      while((numRead = mq_receive(mqd,(char*)buffer,attr.mq_msgsize,NULL)) >= 0)
        printf("Real %ld bytes\n",(long)numRead);
      if(errno != EAGAIN)
        errExit("mq_receive");
    }
}
