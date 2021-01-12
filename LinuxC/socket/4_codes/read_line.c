#include "read_line.h"
ssize_t 
readLine(int fd,void* buffer,size_t n){

  if(n<0 || buffer==NULL){
    errno = EINVAL;
    return -1;
  }
  char* buf =(char*) buffer;
  size_t totRead = 0;
  ssize_t numRead;
  char c;
  for(;;){
    numRead = read(fd,&c,1);
    if(numRead == -1){
      if(errno == EINTR)//Interrupted --> restart
        continue; 
      else //other error 
        return -1;
    }else if(numRead == 0){ //EOF
        if(totRead == 0)
          return 0; //No bytes read 
        else break; //some bytes read.break to add \0
    }else{
      //if number of bytes read before \n is encountered is >= n-1,then discards excess bytes,include \n
      //if \n is read within n-1 bytes,then it is included in the return string 
      //so we can determine if bytes are discarded by checking if \n procedes terminating null byte in buffer 
      if(totRead < n-1){
        ++totRead;
        *buf++ = c;
      }
      if(c == '\n')
        break;
    } 
  }
  //add \0 and return 
  *buf = '\0';
  return totRead;
}
