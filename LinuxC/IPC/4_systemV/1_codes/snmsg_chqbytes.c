#include <sys/stat.h>
#include <sys/msg.h>
#include "tlpi_hdr.h" 

int main(int argc,char* argv[]){
  if(argc != 3 || strcmp(argv[1],"--help") ==0)
    usageErr("%s msqid max-bytes\n",argv[0]);

  //retrieve copy of associate data structure from kernel
  struct msqid_ds ds;
  int msqid;
  if(msgctl(msqid,IPC_STAT,&ds) == -1)
    errExit("msgctl");

  //update 
  ds.msg_qbytes = getInt(argv[2],0,"max-bytes");
  if(msgctl(msqid,IPC_SET,&ds) == -1)
    errExit("msgctl");
  exit(EXIT_SUCCESS);
}
