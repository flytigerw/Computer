#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

static void notifySetup(mqd_t* mqd);

//Thread notification function
static void threadFunc(union sigval sv){
  mqd_t* mqd = (mqd_t*)sv.sival_ptr;
  struct mq_attr attr;
  if(mq_getattr(*mqd,&attr) == -1)
    errExit("mq_getattr");

  void* buffer =malloc(attr.mq_msgsize);
  if(buffer == NULL)
    errExit("malloc");
  

  //Register the process again after the notification has arrived
  notifySetup(mqd);
  
  ssize_t numRead;
  while((numRead = mq_receive(*mqd,(char*)buffer,attr.mq_msgsize,NULL) >= 0))
    printf("Read %ld bytes\n",(long)numRead);
  
  //Unexpected error
  if(errno != EAGAIN)
    errExit("mq_receive");

  free(buffer);
  pthread_exit(NULL);
}

static void notifySetup(mqd_t* mqd){
  struct sigevent sev;
  sev.sigev_notify = SIGEV_THREAD;
  sev.sigev_notify_function = threadFunc;
  sev.sigev_notify_attributes = NULL;
  
  sev.sigev_value.sival_ptr = mqd;
  if(mq_notify(*mqd,&sev) == -1)
    errExit("mq_notify");
}
int main(int argc,char* argv[]){
  if(argc != 2 || strcmp(argv[1],"--help") == 0)
    usageErr("%s mq-name\n",argv[0]);
  
  mqd_t mqd = mq_open(argv[1],O_RDONLY | O_NONBLOCK);
  if(mqd == (mqd_t)-1)
    errExit("mq_open");
  
  //register the process 
  notifySetup(&mqd);
  //wait for notification 
  //Timer notifications are delivered by invocations of threadFunc() in a separate thread
  pause();
}
