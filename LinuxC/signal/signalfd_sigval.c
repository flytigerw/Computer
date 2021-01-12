
#include <sys/signalfd.h>
#include <signal.h>
#include "tlpi_hdr.h"

int main(int argc, char* argv[]){
   if(argc < 2)
    usageErr("%s:sig-num...\n",argv[0]);
   
   //show pid and uid
   printf("%s:PID is %ld, UID is %ld\n",argv[0],(long)getpid(),(long)getuid());

   //block sigs that are assigned in arguments 
   sigset_t mask;
   sigemptyset(&mask);
   for(int i=1; i<argc;++i)
     sigaddset(&mask,atoi(argv[i]));
   if(sigprocmask(SIG_BLOCK,&mask,NULL) == -1)
     errExit("sigprocmask");

   //set fd to read sigs 
   int sfd = signalfd(-1,&mask,0);
   if(sfd == -1)
     errExit("signalfd");
   
   
   //read sigs from sfd
   struct signalfd_siginfo sfdBuf;
   for(;;){
    ssize_t s = read(sfd,&sfdBuf,sizeof(struct signalfd_siginfo));
    if(s != sizeof(struct signalfd_siginfo))
      errExit("read");
    
    printf("%s:got signal %d\n",argv[0],sfdBuf.ssi_signo);
    if(sfdBuf.ssi_code == SI_QUEUE){
      printf("; ssi_pid = %d ",sfdBuf.ssi_pid);
      printf("ssi_int = %d",sfdBuf.ssi_int);
    }
    printf("\n");
   }
}
