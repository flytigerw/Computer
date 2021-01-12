
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

int main(int argc,char* argv[]){
  
  //参数检查
  if(argc != 3)
    usageErr("%s oldFile,newFile\n",argv[0]);

  //打开原文件
  int oldFd = open(argv[1],O_RDONLY);
  if(oldFd == -1)         //确保源文件存在
    errExit("open oldFd");

  //打开新文件
  int openFlags = O_RDWR | O_CREAT | O_TRUNC;
  mode_t perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; //rw-rw-rw 
  int newFd = open(argv[2],openFlags,perms);
  if(newFd == -1)
    errExit("open newFd");

  //buffer准备
#define BUF_SIZE 1024
  char buf[BUF_SIZE];

  //以buffer作为中介
  int numRead;
  while((numRead = read(oldFd,buf,BUF_SIZE)) >0 )   //为0表示读取到文件尾
    if(write(newFd,buf,numRead) != numRead)
      fatal("write");
  //读取出错的检查
  if(numRead == -1)
    errExit("read");

  //文件关闭
  if(close(oldFd) == -1)
    errExit("close");
  //文件关闭
  if(close(newFd) == -1)
    errExit("close");
  exit(EXIT_SUCCESS);
}
