#include <sys/types.h>
#include <sys/sem.h>
#include <time.h>
#include "semun.h"
#include "tlpi_hdr.h"

int main(int argc,char* argv[]){
  if(argc != 2)
    usageErr("%s semid\n",argv[0]);
  
  int semid = getInt(argv[1],0,"semid");
  union semun arg,dummy;
  struct semid_ds ds;

  arg.buf = &ds;
  if(semctl(semid,0,IPC_STAT,arg) == -1)
    errExit("semctl");
  printf("Semaphore changed: %s",ctime(&ds.sem_ctime));
  printf("Last semop():      %s",ctime(&ds.sem_otime));

  //Display per-semaphore information
  arg.array = (unsigned short*)calloc(ds.sem_nsems,sizeof(arg.array[0]));
  if(arg.array == NULL)
    errExit("calloc");
  if(semctl(semid,0,GETALL,arg) == -1)
    errExit("semctl,GETALL");

  printf("Sem # Value SEMPID  SMENCNT  SEMZCNT\n");
  for(int i=0;i<(int)ds.sem_nsems;i++){
    printf("%3d   %5d   %5d   %5d   %5d\n",i,arg.array[i],semctl(semid,i,GETPID,dummy),
                                                          semctl(semid,i,GETNCNT,dummy),
                                                          semctl(semid,i,GETZCNT,dummy));
  }
  exit(EXIT_SUCCESS);
}
