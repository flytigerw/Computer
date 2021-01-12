

#define  _GNU_SROUCE
#include <signal.h>
#include "tlpi_hdr.h"
#include <string.h>

static void 
sigsegvHandler(int sig){
  int x;
  printf("Catch signal %d (%s)\n",sig,strsignal(sig));
  printf("Top of the handler stack near   %10p\n",(void*)&x);

  //清理后，终止进程
  fflush(NULL);
  _exit(EXIT_FAILURE);
}


static void 
overflowStack(int callNum){  //a recursive function that overflows the stack 
   char a[100000000];
   printf("Call %4d - top of stack near %10p\n",callNum,&a[0]);
   overflowStack(callNum+1);
}
  

int main(int argc,char* argv[]){
  int j;
  printf("top of the standard stack near %10p\n",(void*)&j);

  //allocate alternate stack and inform the kernal 
  stack_t sigstack;
  sigstack.ss_sp = malloc(SIGSTKSZ);
  if(sigstack.ss_sp == NULL)
    errExit("malloc");
  sigstack.ss_size=SIGSTKSZ;
  sigstack.ss_flags=0;

  if(sigaltstack(&sigstack,NULL) == -1)
    errExit("sigaltstack");

  printf("Alternate stack is at   %10p-%p\n",sigstack.ss_sp,(char*)sbrk(0)-1);

  //install handler for SIGSEGV
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_ONSTACK;
  sa.sa_handler = sigsegvHandler;
  if(sigaction(SIGSEGV,&sa,NULL) == -1)
    errExit("sigactiong");
  
  overflowStack(1);


}
