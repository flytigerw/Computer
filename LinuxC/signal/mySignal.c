

#include <signal.h>
//signal的实现 --->调用sigaction

typedef void(* sighandler_t)(int);

sighandler_t 
signal(int sig, sighandler_t handler){
  struct sigaction newSa,oldSa;
  newSa.sa_handler = handler;
  sigemptyset(&newSa.sa_mask);
//兼顾历史遗留
#ifndef OLD_SIGNAL
  newSa.sa_flags= SA_RESETHAND | SA_NODEFER ;
#else 
  newSa.sa_flag= SA_RESTART;
#endif
  if(sigaction(sig,&newSa,&oldSa) == -1)
    return SIG_ERR;
  else 
    return oldSa.sa_handler;
}
