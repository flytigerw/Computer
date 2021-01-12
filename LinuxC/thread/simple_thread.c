

#include <pthread.h>
#include "tlpi_hdr.h"

static void *
threadFunc(void* arg){
  char* s = (char*)arg;
  printf("%s",s);
  return (void*)strlen(s);
}

int main(int argc,char* argv[]){
  pthread_t t1;
  int s = pthread_create(&t1,NULL,threadFunc,"HelloWorld\n");
  if(s != 0)
    errExitEN(s,"pthread_create");
  
  printf("Message from main()\n");
  void* res;
  s = pthread_join(t1,&res);
  if(s != 0)
    errExitEN(s,"pthread_join");

  printf("Thread returned %ld\n",(long)res);

  exit(EXIT_SUCCESS);
}
