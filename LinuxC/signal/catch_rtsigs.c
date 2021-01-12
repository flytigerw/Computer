

#define _GNU_SOURCE
#include "tlpi_hdr.h"
#include <string.h>
#include <signal.h> 

//arguments： sleepTime
//休眠等待RTsignals 到来。对RTsignals做排队处理，观察解除信号阻塞时的情况

static volatile int sigCnt=0;
static volatile int allDone=0;
static volatile int handerSleepTime;   //放缓程序的执行，有助于看清处理多个信号所发生的的情况

static void //handelrs for signals established using SA+SIGINFO
handler(int sig, siginfo_t* siginfo, void* ucontext){
   //SIGINT or SIGTERM can term the program 
  if(sig == SIGINT || sig== SIGTERM){
     allDone=1;
     return ;
  }

  sigCnt++;
  printf("Catch signal %d\n",sig);
  //show siginfo 
  printf("  si_siginfo=%d, si_code=%d (%s),", siginfo->si_signo, siginfo->si_code,
          siginfo->si_code==SI_USER ? "SI_USER" : (siginfo->si_code==SI_QUEUE) ? "SI_QUEUE" : "other");
  printf("si_value=%d\n,",siginfo->si_value.sival_int);
  printf("  si_pid=%ld  si_uid=%ld \n",(long)siginfo->si_pid,(long)siginfo->si_uid);
  
  sleep(handerSleepTime);
}

int main(int argc, char* argv[]){
  if(argc >1 && strcmp(argv[1],"--help")==0)
    usageErr("%s [block-time [handler-sleep-tiem]]\n",argv[0]);
  
  printf("%s: PID is %ld\n",argv[0],(long)getpid());
  
  handerSleepTime = (argc>2)? getInt(argv[2],GN_NONNEG,"Handler-sleep-time"):1;
 
  //install handlers for most sigs 
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sigfillset(&sa.sa_mask);                //prevent handlers recursively interrupting each other
  sa.sa_sigaction = handler;
  for(int sig=1; sig<NSIG; sig++)
    if(sig != SIGTSTP && sig != SIGQUIT)
      sigaction(sig,&sa,NULL);

  //optioanlly block sigs and allowing sig to be received before they are unblocked and handled 
  if(argc > 1){
     sigset_t preMask,blockMask;
     sigfillset(&blockMask);
     sigdelset(&blockMask,SIGINT);
     sigdelset(&blockMask,SIGTERM);

     if(sigprocmask(SIG_SETMASK,&blockMask,&preMask) == -1)
       errExit("sigprocmask");
     
     printf("%s:signals blocked - sleeping %s secondes\n",argv[0],argv[1]);
     sleep(getInt(argv[1],GN_GT_0,"block-time"));
     printf("%s:sleep complete\n",argv[0]);

     //restore sig mask 
     if(sigprocmask(SIG_SETMASK,&preMask,NULL) == -1)
       errExit("sigprocmask");
  }
   while(!allDone) //wait for incoming signals
     pause();

}
