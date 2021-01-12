#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/msg.h>
#include "tlpi_hdr.h"

int main(int argc,char* argv[]){
  //Obtain maxInde of entries array 
  int maxIdx;
  struct msginfo msgInfo;
  maxIdx = msgctl(0,MSG_INFO,(struct msqid_ds*)&msgInfo);
  if(maxIdx == -1)
    errExit("msgctl-MSG_INFO");

  printf("maxIdx: %d\n\n",maxIdx);
  printf("index     id      key      messages\n");
  
  //retrieve and display informaion for each item of the entries array 
  struct msqid_ds ds;
  int msqid;
  for(int i=0;i<maxIdx;i++){
    msqid = msgctl(i,MSG_STAT,&ds);
    if(msqid == -1){
      if(errno != EINVAL && errno != EACCES)
        errMsg("msgctl_MSG_STAT");
      continue;
    }
    printf("%4d %8d  0x%08lx %7ld\n,",i,msqid,(unsigned long)ds.msg_perm.__key,(long)ds.msg_qnum);
  }
  exit(EXIT_SUCCESS);
}
