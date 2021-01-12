

#include "tlpi_hdr.h"

int main(int argc, char* argv[]){
  if(argc != 2)
    usageErr("%s:pathname\n",argv[0]);

  //get basename of argv[1]
  char* argVec[10];
  argVec[0] = strrchr(argv[1],'/');

  if(argVec[0] != NULL)
    argVec[0]++;
  else 
    argVec[0]=argv[1];

  argVec[1]="hello world";
  argVec[2]="good bye";
  argVec[3]=NULL;

  char* envVec[]={"GREET=sault","BYE=adieu",NULL};

  execve(argv[1],argVec,envVec);
  errExit("execve");

}
