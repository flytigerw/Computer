
#include "tlpi_hdr.h"
#define MAX_ALLOCS 100000

int main(int argc,char* argv[]){
  if(argc < 3)
    usageErr("wrong arguments");
  //解析出命令行中的数字
  int numAllocs = getInt(argv[1],GN_GT_0,"num-allocs");
  if(numAllocs > MAX_ALLOCS)
    cmdLineErr("num-allocs >%d\n",MAX_ALLOCS);

  int blockSize = getInt(argv[2],GN_GT_0|GN_ANY_BASE,"block-size");
  int freeStep = (argc>3)? (getInt(argv[3],GN_GT_0,"step")) : 1;
  int freeMin  = (argc>4)? (getInt(argv[4],GN_GT_0,"min")) : 1;
  int freeMax  = (argc>5)? (getInt(argv[5],GN_GT_0,"max")) : numAllocs;
  
  if(freeMax > MAX_ALLOCS)
    cmdLineErr("free-max > MAX_ALLOCS\n");

  //开始追踪内存的分配情况
  printf("Inital program break:    %10p\n",sbrk(0));

  char* ptr[MAX_ALLOCS];      //存放分配的内存的地址

  for(int i=0;i<numAllocs;i++){
       ptr[i]=(char*)malloc(blockSize);
       if(ptr[i] == NULL)
         errExit("malloc");
  }

  printf("Program break now is:    %10p\n",sbrk(0));
  
  printf("Freeing Blocks from %d to %d\n ",freeMin,freeMax);
  
  for(int j=freeMin-1;j<freeMax; j+=freeStep)
    free(ptr[j]);

  printf("After free,the program break is:     %10p\n",sbrk(0));
  exit(EXIT_SUCCESS);
}
