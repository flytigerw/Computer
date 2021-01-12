
#include <pthread.h>
#include "tlpi_hdr.h"

static void* 
threadFunc(void* arg){
  printf("New thread started\n");

  for(int i=1;;i++){
    printf("Loop %d\n",i);        //stdion may be a cancellation point 
    sleep(1);                    //cancellation point 
  }

  return NULL;
}

int main(int argc,char* argv[]){
  //created thread 
  pthread_t tid;
  int s = pthread_create(&tid,NULL,threadFunc,NULL );
  if(s != 0)
    errExitEN(s,"pthread_create");
  

  //allow new thread to run for a while 
  sleep(3);

  //send cancel req 
  s = pthread_cancel(tid);
  if(s != 0)
    errExitEN(s,"pthread_cancel");

  //join the thread
  void* res;
  s = pthread_join(tid,&res);
  if(s != 0)
    errExitEN(s,"pthread_join");

  //judge the res 
  if(res == PTHREAD_CANCELED) 
    printf("Thread was canceled\n");
  else 
    printf("Thread was not canceled\n");

  exit(EXIT_SUCCESS);
}
