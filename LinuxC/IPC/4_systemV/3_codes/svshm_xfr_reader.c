#include "svshm_xfr.h"

int main(int argc,char* argv[]){
  int semid = semget(SEM_KEY,0,0);
  if(semid == -1)
    errExit("semid---");
  int shmid = shmget(SHM_KEY,0,0);
  if(shmid == -1)
    errExit("shmget");
  struct shmseg* shmp;
  shmp =(struct shmseg*) shmat(shmid,NULL,SHM_RDONLY);
  if(shmp == (void*)-1)
      errExit("shmat");
  //Transfer blocks of data from shared memory to stdout 
  int i,bytes;
  for(i=0,bytes=0;;i++){
      if(reserveSem(semid,READ_SEM) == -1)
        errExit("reserveSem");
      if(shmp->cnt == 0)     //Writer encounter EOF
        break;
      bytes += shmp->cnt;
      if(write(STDOUT_FILENO,shmp->buf,shmp->cnt) != shmp->cnt)
       fatal("partial/failed write");
      if(releaseSem(semid,WRITE_SEM) == -1)
        errExit("releaseSem");
  }

  if(shmdt(shmp) == -1)
    errExit("shmdt");
  if(releaseSem(semid,WRITE_SEM) == -1)
    errExit("releaseSem");
  
  fprintf(stderr,"Received %d bytes(%di)\n",bytes,i);
  exit(EXIT_SUCCESS);
}
