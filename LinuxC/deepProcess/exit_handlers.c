

#define _BSD_SOURCE
#include <stdlib.h>
#include "tlpi_hdr.h"

static void 
atHandler1(void){
  printf("atexit handler1 called\n");
}

static void 
atHandler2(void){
  printf("atexit handler2 called\n");
}

static void 
onHandler1(int exitStatus,void* arg){
  printf("on_exit handler called:status=%d, arg=%ld\n",exitStatus,(long)arg);
}

int main(int argc, char* argv[]){
 if(on_exit(onHandler1,(void*)10) != 0)
   fatal("on_exit1");
 if(atexit(atHandler1) != 0)
   fatal("on_exit1");
 if(atexit(atHandler2) != 0)
  fatal("on_exit2");
 exit(2);


}
