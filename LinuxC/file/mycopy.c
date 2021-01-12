
#include <sys/sdt.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#ifndef BUF_SIZE                  //也允许在编译时通过 gcc -D 来overide definition
#define BUF_SIZE 1024
#endif 

/*copy流程
 *1验证命令行参数的有效-->个数验证:3个
 *2source file必须存在且能够打开
 *3 dest  file不一定存在，可以创建。
 *            若要创建，则要指定flags以及新建文件的mode
 *4读取source file到buffer中
 * 再将buffer中的数据写入到dest file中
 *5释放close(fd)
 * */

int main(int argc,char* argv[]){

  if(argc != 3)
    usageErr("cmd is not right");

  int inputFd = open(argv[1],O_RDONLY);
  if(inputFd == -1)
    errExit("fail to open file:%s",argv[1]);

  int openFlags = O_CREAT | O_WRONLY | O_TRUNC;    //创建新文件或者截断旧文件，再只写入
  int mode=S_IRUSR | S_IWUSR | 
           S_IRGRP | S_IWGRP |
           S_IROTH | S_IWOTH ;     //rw-rw-rw 
  int outFd = open(argv[2],openFlags,mode);
  if(outFd == -1)
    errExit("fail to open or create dest file:%s",argv[2]);

  int numRead;
  char buffer[BUF_SIZE];
  while((numRead=read(inputFd,buffer,BUF_SIZE))>0 ){        //=0表示读取结束
    if(write(outFd,buffer,numRead) != numRead )
      fatal("couldn't write the whole buffer");
  }
  if(numRead == -1)
   errExit("read error");
  if(close(inputFd) == -1)
    errExit("fail to close source file:%s",argv[1]);
  if(close(outFd) == -1)
    errExit("fail to close dest file:%s",argv[1]);

  exit(EXIT_SUCCESS);
}
