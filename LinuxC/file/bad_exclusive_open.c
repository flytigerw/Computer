

#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

int main(int argc,char* argv[]){
  
  int fd = open(argv[1],O_WRONLY);

  if(fd != -1){    //open succeeded
    printf("[PID %ld] File \"%s\" already exits\n",(long)getpid(),argv[1]);
    close(fd);
  }else{           //open fail  

    if(errno != ENOENT){   //fail for unexpected reason
      errExit("open");
    }else{     //fail for file not exiting   what if time is up and another process create the file ?

      printf("[PID %ld] File \"%s\" doesn't exist yet\n",(long)getpid(),argv[1]);
      //make it sleep and make another process create the file
      if(argc >2){
        sleep(5);
        printf("[PID %ld] Done sleep\n",(long)getpid());
      }

      //open or create
      fd = open(argv[1],O_WRONLY | O_CREAT|O_EXCL,S_IRUSR|S_IWUSR);   //O_CREAT without O_EXCL--->open succeed once there is the file.but it may not be create by ownself
      if(fd == -1)
        errExit("open");
      printf("[PID %ld] Create file: \"%s\"",(long)getpid(),argv[1]);
    }
  }


  exit(EXIT_SUCCESS);
}
