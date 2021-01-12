#ifndef BINARY_SEMS_H
#define BINARY_SEMS_H 

#include "tlpi_hdr.h"
//Variables controlling the operation 
extern Boolean bsUseSemUndo;          //Use SEM_UNDO during semop()
extern Boolean bsRetryOnEintr;        //Retry if semop() interrupted by signal handler?

int initSemAvailable(int semld,int semNum);
int initSemInUse(int semld,int semNum);
int reserveSem(int semld,int semNum);
int releaseSem(int semId,int semNum);
#endif 
