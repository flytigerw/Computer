#include <unistd.h> 
#include "stdio.h"
#include "unistd.h"
#include "tlpi_hdr.h"

int main(){
  #define  MAX_READ 10
  char buffer[MAX_READ];
  
  if(read(STDIN_FILENO,buffer,MAX_READ) == -1)
    errExit("read");
  printf("the input data was:%s\n",buffer);
  exit(EXIT_SUCCESS);
}
