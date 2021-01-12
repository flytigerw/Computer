


//creates a thread and joins with it 
#include <pthread.h>
#include "tlpi_hdr.h"

static void* thread_func(void* arg){
  char* s= (char*) arg;         //调用者给出具体实现
  printf("%s\n",s);
  return (void*)strlen(s);
}

int main(int argc,char* argv[]){
  //create thread
  pthread_t id;
  int s = pthread_create(&id,NULL,thread_func,"hello wolrd\n");
  if(s != 0)
    errExitEN(s,"pthread_create");

  printf("messages from main\n");

  //joins it 
  void* res;
  s = pthread_join(id,&res);
  if(s != 0)
    errExitEN(s,"pthread_join");

  //get the retval 
  printf("Thread returned %ld\n",(long)res);
  exit(EXIT_SUCCESS);
}
