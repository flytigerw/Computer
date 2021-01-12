

#include <sys/wait.h>
#include "tlpi_hdr.h"
#include "print_wait_status.h"

#define  MAX_CMD_LEN 200

int main(int argc,char* argv[]){
   char str[MAX_CMD_LEN];    //cmds to be executed by sysem
   int status;

   for(;;){//read and execute cmds 
     printf("CMD:");
     fflush(stdout);

     if(fgets(str,MAX_CMD_LEN,stdin) == NULL)
       break;    //end of file 

     status = system(str);
     printf("system() returned: status=0x%04x (%d,%d)\n",(unsigned int)status,status>>8,status & 0xff);

     if(status == -1){
       errExit("system");
     }else{
       if(WIFEXITED(status) && WEXITSTATUS(status) == 127)
         printf("(Probably) couldn't invoke shell\n");
       else printWaitStatus(NULL,status);
     }
   }
   exit(EXIT_SUCCESS);
}
