#include "semun.h"
#include "svshm_xfr.h"

int main(int argc,char* argv[]){
  int semid = semget(SEM_KEY,2,IPC_CREAT | OBJ_PERMS );
  if(semid == -1)
    errExit("semget");
  if(initSemAvailable(semid,WRITE_SEM) == -1)
    errExit("initSemAvailable");
  if(initSemInUse(semid,READ_SEM) == -1)
    errExit("initSemInUse");

  int shmid = shmget(SHM_KEY,sizeof(struct shmseg),IPC_CREAT | OBJ_PERMS);
  if(shmid == -1)
    errExit("shmget");

  struct shmseg* shmp;
  shmp =(struct shmseg*)shmat(shmid,NULL,0);
  if(shmp == (void*)-1)
    errExit("shmat");
  //Transfer blocks of data from stdin to shared memory
  int bytes;
  int i;
  for(i=0,bytes=0; ;i++,bytes += shmp->cnt){
    if(reserveSem(semid,WRITE_SEM) == -1)
      errExit("reserveSem");
    shmp->cnt = read(STDIN_FILENO,shmp->buf,BUF_SIZE);
    if(shmp->cnt == -1)
      errExit("read");
    if(releaseSem(semid,READ_SEM) == -1)
      errExit("releaseSem");

    //Test if we have reached EOF
    if(shmp->cnt == 0)
      break;
  }
   //Wait until reader has let has one more turn 
   //We then know reader has finished and so we can delete the IPC objects
   if(reserveSem(semid,WRITE_SEM) == -1)
     errExit("reserveSem");
   union semun dummy;
   if(semctl(semid,0,IPC_RMID,dummy) == -1)
     errExit("semctl");
   if(shmdt(shmp) == -1)
     errExit("shmdt");
   if(shmctl(shmid,IPC_RMID,0) == -1)
     errExit("shmctl");

   fprintf(stderr,"Sent %d bytes\n",bytes);
   exit(EXIT_SUCCESS);
}
