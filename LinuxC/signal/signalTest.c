#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

static int sigCount=0;

void (*oldHandler)(int);

void sigIntHandler(int sig){
  //调用一次handler后会恢复默认
  signal(SIGINT,sigIntHandler);
  ++sigCount;
}
int main(){
  oldHandler = signal(SIGINT,sigIntHandler);

  //键入字符，并键入C-c --> 不会终止程序
  char c;
  while((c=getchar()) != '\n');
  
  printf("catch SIGINT %d times",sigCount);

  signal(SIGINT,oldHandler);

  while((c=getchar()) != '\n');

  exit(EXIT_SUCCESS);
}
