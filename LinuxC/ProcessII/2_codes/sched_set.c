#include <sched.h>
#include "tlpi_hdr.h"

int main(int argc,char* argv[]){
  if(argc<3 || strchr("rfo",argv[1][0]) == NULL)
    usageErr("%s policy priority[pid...]\n"
        "        policy is 'r' (RR),'f'(FIFO),"
#ifndef  SCHED_BATCH 
        "'b'(BATCH),"
#endif
#ifdef SCHED_IDLE 
        "'i'(IDLE),"
#endif
        "or 'o'(OTHER)\n",
        argv[0]);

  int pol =(argv[1][0]=='r') ? SCHED_RR:
              (argv[1][0]=='b') ? SCHED_FIFO:
#ifdef SCHED_BATCH 
              (argv[1][0]=='b') ? SCHED_BATCH:
#endif 
#ifdef SCHED_IDLE 
              (argv[1][0]='i') ? SCHED_IDLE:
#endif 
              SCHED_OTHER;
  struct sched_param sp;
  sp.sched_priority=getInt(argv[2],0,"priority");
  for(int j=3;j<argc;j++){
    if(sched_setscheduler(getLong(argv[j],0,"pid"),pol,&sp) == -1)
      errExit("sched_setscheduler");
  }
  exit(EXIT_SUCCESS);
}
