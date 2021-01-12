
#include <semaphore.h>
#include <pthread.h>
#include "tlpi_hdr.h"

static int glob = 0;
static sem_t sem;

//Loop arg times and increase the glob 
static void* threadFunc(void* arg){
  int loops =*((int*)arg);
  int loc;
  for(int i=0;i<loops;i++){
    if(sem_wait(&sem) == -1)
      errExit("sem_wait");
    loc = glob;
    loc++;
    glob = loc;
    if(sem_post(&sem) == -1)
      errExit("sem_post");
  }
  return  NULL;
}


int main(int argc,char* argv[]){
  pthread_t t1,t2;
  int loops=(argc > 1)? getInt(argv[1],GN_GT_0,"num-loops") : 10000000;
  
  //Initialize a thread-shared mutex 
  if(sem_init(&sem,0,1) == -1)
    errExit("sem_init");
  //Create two threads 
  int s = pthread_create(&t1,NULL,threadFunc,&loops);
  if(s != 0)
    errExitEN(s,"pthread_create");
  s = pthread_create(&t2,NULL,threadFunc,&loops);
  if(s != 0)
    errExitEN(s,"pthread_create");

  //wait for threads to terminate
  s = pthread_join(t1,NULL);
  if(s != 0 )
    errExitEN(s,"pthread_join");
  s = pthread_join(t2,NULL);
  if(s != 0 )
    errExitEN(s,"pthread_join");
  
  printf("glob= %d\n",glob);
  exit(EXIT_SUCCESS);
}
