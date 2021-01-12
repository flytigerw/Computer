
/* 程序要求 
 * 命令行格式: ./seekio filename op1 op2 ..
 * op：
 * 1 soffset:从文件开始检索到offset处
 * 2 rlength:从当前文件偏移量开始,从文件中读取length字节数据,以文本形式显示
 * 3 Rlength:以16进制显示
 * 4 wstr   :从当前文件偏移量开始,写入str
 * */
/* 流程
 * 1 判断参数有效性
 * 2 打开或者创建文件
 * 3 遍历命令行，解析op，执行相应操作
 * 4 
 * */
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"
#include <ctype.h>

int main(int argc,char* argv[]){
   
  if(argc<3)
    usageErr("arguments are not enough");

  int fd=open(argv[1],
              O_RDWR | O_CREAT,
              S_IRUSR | S_IWUSR |
              S_IRGRP | S_IWGRP |      
              S_IROTH | S_IWOTH );
   if(fd == -1)
     errExit("open");
   char* buf; 
   int len;
   int numRead;
   int numWrite;
   int offset;
   for(int i=2; i<argc;i++){
     switch(argv[i][0]){
        case 's' :      //parse out the offset and change fileoffset to offset
          offset=getLong(&argv[i][1],GN_ANY_BASE,argv[i]);
          if(lseek(fd,offset,SEEK_SET) == -1)
            errExit("lseek");
          //give some message
          printf("%s:seek succeeded\n",argv[i]);
          break;
        case 'r':          //it's similar to R,so put in the R solution
        case 'R':
          len=getLong(&argv[i][1],GN_ANY_BASE,argv[i]);
          buf = (char*)malloc(len);  //buf需要保持数据
          if(buf == NULL)
            errExit("malloc");
          numRead= read(fd,buf,len);
          if(numRead==-1)
            errExit("read");
          //read successfully
          if(numRead ==0){
            printf("%s:end of file",argv[i]);
          }
          //display it 
          else{
            printf("%s:",argv[i]);
            //scan and printf  why? because the char may be unprintable
            for(int j=0;j<numRead;j++){
              if(argv[i][0] == 'r')
                printf("%c",isprint((unsigned char)buf[i]) ? buf[j] : '?');
              else 
                printf("%02x",(unsigned int)buf[j]);
              printf("\n");
            }
          }
          break;
       case 'w':
          numWrite=write(fd,&argv[i][1],strlen(&argv[i][1]));
          if(numWrite == -1)
            errExit("write");
          printf("%s:wrote %ld bytes\n",argv[i],(long)numWrite);
          break;

        default:
          //命令行解析出错
          cmdLineErr("arguments are not correct:%s\n",argv[i]);
     }
   }


  exit(EXIT_SUCCESS);
}
