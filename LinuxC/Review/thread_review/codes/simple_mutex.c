


//创建两个线程，他们共享资源glob,用mutex进行保护

#include <pthread.h>
#include "tlpi_hdr.h"

static int glob = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void* thread_func(void* arg){
  int s,tmp;
  int loops = *((int*)arg);
  for(int i=0;i<loops;i++){
    //lock mutex 
    s = pthread_mutex_lock(&mutex);
    if(s != 0)
      errExitEN(s,"lock");
    //use the shared resources 
    tmp = glob;
    ++tmp;
    glob = tmp;
    //unlokc mutex 
    s = pthread_mutex_unlock(&mutex);
    if(s != 0)
      errExitEN(s,"unlock");
  }
  return NULL;
}

int main(int argc,char* argv[]){
  int loops = (argc > 1) ? getInt(argv[1],GN_GT_0,"num-loops") : 100000;
  //create two threads 
  pthread_t id1,id2;
  int s = pthread_create(&id1,NULL,thread_func,&loops);
  if(s != 0)
    errExitEN(s,"pthread-create");
  s = pthread_create(&id2,NULL,thread_func,&loops);
  if(s != 0)
    errExitEN(s,"pthread-create");
  //join them 
  s = pthread_join(id1,NULL);
  if(s != 0)
    errExitEN(s,"pthread-join");
  s = pthread_join(id2,NULL);
  if(s != 0)
    errExitEN(s,"pthread-join");
  exit(EXIT_SUCCESS);
}
