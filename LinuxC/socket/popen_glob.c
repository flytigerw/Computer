#include <ctype.h>
#include <limits.h>
#include "tlpi_hdr.h"
#include "print_wait_status.h" 

#define POPEN_FMT "/bin/ls -d %s 2>/dev/null"
#define PAT_SIZE 50
#define PCMD_BUF_SIZE (sizeof(POPEN_FMT)+PAT_SIZE)

int main(int argc,char* argv[]){
  char pat[PAT_SIZE];         //Pattern for globbing
  char popenCmd[PCMD_BUF_SIZE];
  FILE* fp;             //returned by popen()
  Boolean badPattern;   //Invaild characters in pat ?
  int len,status,fileCnt;
  char pathname[PATH_MAX];

  for(;;){         //read pattern,display results of globbing
    printf("pattern: ");
    fflush(stdout);
    if(fgets(pat,PAT_SIZE，stdin) == NULL)
      break;                 //EOF
    len = strlen(pat);
    if(len <= 1)    //Empty line
      continue;
    
    if(pat[len-1] == '\n')     //strip trailing newline
      pat[len-1]='\0';

    //Ensure the pattern contains only vaild characters 
    int i;
    for(i=0,badPattern=FALSE; i<len && !badPattern;i++)
      if(!isalnum((unsigned char)pat[i]) && strchr("_*?[^-].",pat[i]) == NULL)
        badPattern = TRUE;
     
    if(badPattern){
      printf("Bad badPattern character: %c\n",pat[i-1]);
      continue;
    }

    //Build and execute CMD to glob pat 
    snprintf(popenCmd,PCMD_BUF_SIZE,POPEN_FMT,pat);
    popenCmd[PCMD_BUF_SIZE-1]='\0';             //Ensure its null-terminated

    fp=popen(popenCmd,"r");
    if(fp == NULL){
      printf("popen() failed\n");
      continue;
    }

    //read resulting list of pathnames until EOF
    fileCnt = 0;
    while(fgets(pathname,PATH_MAX,fp) != NULL){
      printf("%s",pathname);
      fileCnt++;
    }

    //close pipe,fetch and display termination status 
    status = pclose(fp);
    printf("    %d matching files%s\n",fileCnt,(fileCnt != 1)? "s":"");
    printf("    pclose() status=%#x\n",(unsigned int)status);
    if(status != -1)
      printWaitStatus("\t",status);
  }
  exit(EXIT_SUCCESS);
}
