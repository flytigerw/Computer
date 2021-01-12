
#include <pthread.h>
#include "tlpi_hdr.h"

static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static int glob=0;

//cleanup handler: Free memory and unlock mutex 
static void 
cleanupHandler(void* arg){
  printf("cleanup: freeing block at %p\n",arg);
  free(arg);

  printf("cleanup: unlocking mutex\n");
  int s = pthread_mutex_unlock(&mtx);
  if(s != 0)
    errExitEN(s,"pthread_mutex_unlock");
}

static void* 
threadFunc(void* arg){

  //created Buffer 
  char* buf = NULL;
  buf = (char* )malloc(0x10000);
  printf("thread: allocated memory at %p\n",buf);

  //wait at glob to cancle or  cleanup 
  int s = pthread_mutex_lock(&mtx);
  if(s != 0)
    errExitEN(s,"pthread_mutex_lock");
  
  pthread_cleanup_push(cleanupHandler,buf);
  while(glob == 0){
    s = pthread_cond_wait(&cond,&mtx);   //cancellation point
    if(s != 0)
      errExitEN(s,"pthread_cond_wait");
  }
  
  printf("thread: conditon wait loop completed\n");
  pthread_cleanup_pop(1);
  return NULL;
}

int main(int argc,char* argv[]){
  //created thread 
  pthread_t tid; 
  int s = pthread_create(&tid,NULL,threadFunc,NULL);
  if(s != 0)
    errExitEN(s,"pthread_create");

  //give thread a chance to statrt 
  sleep(2);

  if(argc == 1){ //cancel thread 
    printf("main: about to cancel thread\n");
    s = pthread_cancel(tid);
    if(s != 0)
      errExitEN(s,"pthread_cancel");
  }else{
    //signal cond 
    printf("main: about to signal cond\n");
    glob = 1;
    s = pthread_cond_signal(&cond);
    if(s != 0)
      errExitEN(s,"pthread_cond_signal");
  }
  //join thread 
  void* res;
  s = pthread_join(tid,&res);
  if(s != 0)
    errExitEN(s,"pthread_join");
  if(res == PTHREAD_CANCELED)
    printf("main: thread was canceled\n");
  else 
  printf("main: thread was not canceled\n");

  exit(EXIT_SUCCESS);
}
