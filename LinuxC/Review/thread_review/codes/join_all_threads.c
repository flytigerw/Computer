#include <pthread.h>
#include "tlpi_hdr.h"


enum thread_state{
  TS_ALIVE,
  TS_TERMINATE,            //Thread terminated,yet not joined
  TS_JOINED
};

static struct{
  pthread_t id;
  enum thread_state state;
  int sleep_time;
} *thread_info;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

static int total_nums = 0;         //总的线程数 ----> 用于确定thread_info数组的小
static int alive_nums = 0;         //还活着的线程数
static int unjoined_nums = 0;      //条件变量的测试条件.desired state: unjoined_nums != 0

static void* thread_func(void* arg){  //arg为其在thread_info中的下标，取出sleep_time 
  int idx = *((int*)arg);
  //stimulate working for a period of time 
  sleep(thread_info[idx].sleep_time);
  printf("Thread %d is going to terminate\n",idx);

  //update its info exclusively 
  int s = pthread_mutex_lock(&mutex);
  if(s != 0)
    errExitEN(s,"mutex-lock");

  unjoined_nums++;
  thread_info[idx].state = TS_TERMINATE;

  s = pthread_mutex_unlock(&mutex);
  if(s != 0)
    errExitEN(s,"mutex-unlock");

  //signal the main thread to join the thread 
  s = pthread_cond_signal(&cond_var);
  if(s != 0)
    errExitEN(s,"signal-cond");

  return NULL;
}

int main(int argc,char* argv[]){

  if(argc < 2)
    usageErr("%s sleep_time....",argv[0]);
  //alloc for thread_info 
  thread_info = calloc(argc-1,sizeof(*thread_info));
  if(thread_info == NULL)
    errExit("calloc");
  //create threads 
  int s;
  int idx;
  for(int i=1;i< argc;i++){
    idx = i-1;
    thread_info[idx].sleep_time = getInt(argv[i],GN_NONNEG,NULL);
    thread_info[idx].state = TS_ALIVE;
    printf("%d\n",idx);
    s = pthread_create(&thread_info[idx].id,NULL,thread_func,&idx);
    sleep(1);
    if(s != 0)
      errExitEN(s,"thread-create");
  }
  
  total_nums = argc-1;
  alive_nums = total_nums;

  while(alive_nums > 0){
    //lock mutex for checking exclusively 
    s = pthread_mutex_lock(&mutex);
    if(s != 0)
      errExitEN(s,"mutex_lock_in_main_thread");
  //wait on the cond var 
    while(unjoined_nums == 0){
      s = pthread_cond_wait(&cond_var,&mutex);
      if(s != 0)
        errExitEN(s,"cond_wait");
    }
    //scan the thread array to find out which thread has ternimate yet not joined 
    //then join the thread and update its state 
    for(int i=0;i<argc-1;i++){
      if(thread_info[i].state == TS_TERMINATE){
        s = pthread_join(thread_info[i].id,NULL);
        if(s != 0)
          errExitEN(s,"pthread_join");

        thread_info[i].state = TS_JOINED;

        alive_nums--;
        unjoined_nums--;
        printf("Repead thread %d (num_alive = %d)\n",i,alive_nums);
      }
    }
    //unlock the mutex 
    s = pthread_mutex_unlock(&mutex);
    if(s != 0)
      errExitEN(s,"mutex_unlock");
  }
  exit(EXIT_SUCCESS);
}
