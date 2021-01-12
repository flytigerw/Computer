

#include <string.h>
#include <sys/stat.h>
#include "tlpi_hdr.h"
#include <ctype.h>
#include <fcntl.h>


int main(int argc,char* argv[]){
  
  //参数检查
  if(argc < 3)
    usageErr("%s fileName s(offset)|r(length)|R(length)|w(str)\n",argv[0]);

  //原文件打开或者创建
  int openFlags = O_RDWR | O_CREAT ;
  mode_t perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; //rw-rw-rw 
  int fd = open(argv[1],openFlags,perms);
  if(fd == -1)
    errExit("open");

  //遍历操作
  int i;
  int offset,len;
  ssize_t numRead,numWrite;
  char* buf;         //各次读取的内容大小不一  ---> 动态分配内存
  for(i = 2;i < argc; i++){
    //解析单个操作
    switch(argv[i][0]){
      case 's':
        offset = getLong(&argv[i][1],GN_ANY_BASE,argv[i]);
        if(lseek(fd,SEEK_SET,offset) == -1)
          errExit("lseek");
        printf("%s: seek succeed\n",argv[i]);
        break;
      //r和R有共同操作，只是打印的方式不一样 ---> 将打印的判断放到最后
      case 'r':
      case 'R':
        len = getLong(&argv[i][1],GN_ANY_BASE,argv[i]);

        buf = (char*)malloc(len);
        if(buf == NULL)
          errExit("malloc");

        //读取到buf
        numRead = read(fd,buf,len);
        //读取出错
        if(numRead == -1)
          errExit("read");
        //结尾
        if(numRead == 0){
          printf("%s end-of-file\n",argv[i]);
        }else{
         //打印
          for(int i=0;i<numRead; numRead++){
            if(argv[i][0] == 'r')
              printf("%c",isprint((unsigned char)buf[i]) ? buf[i] : '?'); //打印字符，先检查字符能否打印
            else 
              printf("%02x",(unsigned int)buf[i]);            //16进制打印
          }
        }
        free(buf);
        break;
      case 'w':
        numWrite = write(fd,&argv[i][1],strlen(&argv[i][1]));
        if(numWrite != -1)
          errExit("write");
        else 
          printf("%s succeed wrtie\n",argv[1]);
        break;
      default:    //参数出错
        cmdLineErr("Argument must start with srRw\n");
    }
  }
    exit(EXIT_SUCCESS);
}
