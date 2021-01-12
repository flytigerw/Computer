

#define _GNU_SOURCE
#include <string.h>
#include <signal.h>
#include "tlpi_hdr.h"
#include <time.h>

int main(int argc, char* argv[]){
  if(argc>1 && strcmp(argv[1],"--help")==0)
    usageErr("%s:[delay-seconds]\n",argv[0]);

  //show pid and uid 
  printf("%s: PID is %ld, UID is %ld\n",argv[0],(long)getpid(),(long)getuid());
 
  //block all sigs except SIGKILL and SIGSTOP
  sigset_t allSigs;
  sigfillset(&allSigs);
  if(sigprocmask(SIG_SETMASK,&allSigs,NULL) == -1)
    errExit("sigprocmask");

  printf("%s:signals blocked\n",argv[0]);

  //delay so that sigs can't be sent to su 
  if(argc >1){
    printf("%s:is about to sleep %s seconds\n",argv[0],argv[1]);
    sleep(getInt(argv[1],GN_GT_0,"sleep-time"));
    printf("%s:finished sleeping\n",argv[0]);
  }

  //catch sigs untill catch SIGINT or SIGTERM
  siginfo_t si;
  int sig; 
  for(;;){
   //同步等待
   if((sig=sigwaitinfo(&allSigs,&si)) == -1)
     errExit("sigwaitinfo");

   if(sig == SIGINT || sig == SIGTERM)
      exit(EXIT_SUCCESS);

   printf("got signal: %d(%s)\n",sig,strsignal(sig));
   //show siginfo
   printf("  si_signo=%d,si_code=%d(%s),si_value=%d\n",si.si_signo,si.si_code,
              si.si_code==SI_USER ? "SI_USER": si.si_code==SI_QUEUE ? "SI_QUEUE" : "other",
              si.si_value.sival_int);
   printf("  si_pid=%ld, si_uid=%ld\n",(long)si.si_uid,(long)si.si_pid);
  }

  

  exit(EXIT_SUCCESS);
}
