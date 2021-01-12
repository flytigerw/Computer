

#include <pthread.h>
#include "tlpi_hdr.h"

static pthread_cond_t threadDied = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

//shared variables
static int totThreads = 0;
static int numAlive = 0;    //alibe +  terminated but not yet joined threads 
static int numUnjoined =0;  //terminated but not yet joined threads  

//thread sates 
enum tstate{
  TS_ALIVE,
  TS_TERMINATED,
  TS_JOINED
};

//info about each thread
static struct{
  pthread_t tid;
  enum tstate state;
  int sleeptime;
} *thread;    //数组

static void* 
threadFunc(void* arg){
  int idx =*((int*)arg);
  sleep(thread[idx].sleeptime);   //simulate doing some work 
  printf("Thread %d terminating\n",idx);
  //change the thread's state 
  int s;
  s = pthread_mutex_lock(&mtx);
  if(s != 0)
    errExitEN(s,"pthread_mutex_lock");
  numUnjoined++;
  thread[idx].state = TS_TERMINATED;
  s = pthread_mutex_unlock(&mtx);
  if(s != 0)
    errExitEN(s,"pthread_mutex_lock");
  //to inform that there is terminated but yet joined thread
  s = pthread_cond_signal(&threadDied);
  if(s != 0)
    errExitEN(s,"pthread_cond_signal");

  return NULL;
}

int main(int argc,char* argv[]){
   if(argc < 2)
     usageErr("%s:sleeptim\n",argv[0]);
   
   //alloc memory for array
   thread = calloc(argc-1, sizeof(*thread));
   if(thread == NULL)
     errExit("calloc");
   
   int s;
   //create all threads 
   for(int idx=0;idx<argc-1;idx++){
     thread[idx].sleeptime=getInt(argv[idx+1],GN_NONNEG,NULL);
     thread[idx].state = TS_ALIVE;
     s = pthread_create(&thread[idx].tid,NULL,threadFunc,&idx);
     if(s != 0)
       errExitEN(s,"pthread_create");
   }

   totThreads = argc-1;
   numAlive = totThreads;

   //join with terminated threads 
   while(numAlive > 0){
     //lock mutex 
     s = pthread_mutex_lock(&mtx);
     if(s != 0) 
       errExitEN(s,"pthread_mutex_lock");
     //test cond 
     while(numUnjoined == 0){
       s = pthread_cond_wait(&threadDied,&mtx);
       if(s != 0)
         errExitEN(s,"pthread_cond_wait");
     }
     //scan out the terminated threads and join them
     for(int i=0;i<totThreads;i++){
       if(thread[i].state == TS_TERMINATED){
         s = pthread_join(thread[i].tid,NULL);
         if(s != 0)
           errExitEN(s,"pthread_join");
         thread[i].state = TS_JOINED;
        //other modifies 
         numAlive--;
         numUnjoined--;
         printf("Reaped thread %d\n",i);
       }
     }
    //unlock mutex 
    s = pthread_mutex_unlock(&mtx);
    if(s!=0)
      errExitEN(s,"pthread_mutex_lock");
   }
    exit(EXIT_SUCCESS);

}
