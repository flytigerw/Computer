




#include <stdio.h>
#include <unistd.h>

void foobar(int i){
  printf("Printing from lib.so %d\n",i);
  sleep(-1);
}
