
#include "tlpi_hdr.h"

extern char** environ;

int main(int argc,char* argv[]){
  for(int i=0;i<argc;i++)
    printf("argv[%d] = %s\n",i,argv[i]);

  for(char** evp=environ; *evp != NULL; evp++)
    printf("environ:%s\n",*evp);

  exit(EXIT_SUCCESS);

}
