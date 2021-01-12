#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "binary_sems.h"
#include "tlpi_hdr.h"

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678

#define OBJ_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) //permissions

#define WRITE_SEM 0        //Writer has access to shared memory 
#define READ_SEM 1         //Reader has access to shared memory

#ifndef  BUF_SIZE         //Allow gcc -D to overide definition
#define BUF_SIZE 1024     //Size of transfer buffer
#endif 

struct shmseg{              //Defines structure of shared memory segment
   int cnt;                 //Number of bytes used in buf
   char buf[BUF_SIZE];
};
