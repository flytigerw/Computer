
//用poepn()执行shell cmd进行文件名匹配
#include <ctype.h>
#include <limits.h>
#include "tlpi_hdr.h"

#define  POPEN_FMT "/bin/ls -d %s 2> /dev/null"
#define  BUF_SIZE  50
#define  PCMD_BUF_SIZE (sizeof(POPEN_FMT) + BUF_SIZE)

int main(int argc,char* argv[]){
  //Pattern for globbing
  char pat[BUF_SIZE];
  int len;
  int status;
  Boolean bad_pattern;
  int i;
  char popen_cmd[PCMD_BUF_SIZE];
  FILE* fp;
  char buf[PATH_MAX];
  int num_read;
  for(;;){
    //read patter from stdin and check it
    printf("pattern: ");
    fflush(stdout);
    if(fgets(pat,BUF_SIZE,stdin) == NULL)
      break;
    len = strlen(pat);
    if(len <= 1)   //Empty line
      continue;
    //strip trailing newline
    if(pat[len-1] == '\n')
      pat[len-1] = '\0';

    //Ensure the pattern contains only vaild characters 
    for(i=0,bad_pattern = FALSE;i<len && !bad_pattern; i++)
      if(!isalnum((unsigned char)pat[i]) && strchr("_*?[^-].",pat[i]) == NULL)
        bad_pattern = TRUE;
    if(bad_pattern){
      printf("Bad pattern character:%c\n",pat[i-1]);
      continue;
    }
    //Build the cmd
    //将pat带入POPEN_FMT中的%s,再拷贝到popen_cmd
    snprintf(popen_cmd,PCMD_BUF_SIZE,POPEN_FMT,pat);
    popen_cmd[PCMD_BUF_SIZE - 1] =  '\0';
    //execute the cmd
    fp = popen(popen_cmd,"r");
    if(fp == NULL){
      printf("popen() failed\n");
      continue;
    }
    //读取命令的执行结果
    num_read = 0;
    while(fgets(buf,BUF_SIZE,fp)!=NULL){
      printf("%s\n",buf);
      num_read++;
    }
    //close pipe,fetch and displat termination status 
    status = pclose(fp);
    printf("  %d matching files %s\n",num_read,(num_read != 1) ? "s" :"");
    printf("  pclose() status == %#x\n",(unsigned int)status);
  }
  exit(EXIT_SUCCESS);
}
