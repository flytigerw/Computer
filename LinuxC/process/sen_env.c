
#define _GNU_SOURCE
#include <stdlib.h>
#include "tlpi_hdr.h"

extern char** environ;

int main(int argc, char* argv[]){
  
  clearenv();

  for(int i=1;i<argc;i++)
    if(putenv(argv[i])!=0)
      errExit("putenv:%s",argv[i]);
  
  if(setenv("GREET","Hello World",0) == -1)
      errExit("setenv");

  unsetenv("BYE");

  char** ep;
  for(ep=environ;*ep!=NULL;ep++)
    puts(*ep);

  exit(EXIT_SUCCESS);
}
