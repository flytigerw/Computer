#include <unistd.h>
#include <errno.h>
#include "rdwrn.h"

ssize_t readn(int fd,void* buffer,size_t n){
  char* buf =(char*) buffer;
  
  ssize_t numRead; //number of bytes of last read 
  size_t totRead; //total number of bytes of read so far 
  for(totRead=0;totRead<n;){
    numRead = read(fd,buf,n-totRead);
    if(numRead == 0) //EOF
      return totRead;
    if(numRead == -1){
        if(errno == EINTR)
          continue;  //signal interrupt-->restart
        else 
          return  -1;
    }
    totRead += numRead;
    buf += numRead;
  }
  return totRead;
}

ssize_t writen(int fd,const void*buffer,size_t n){
  const char* buf =(const char*) buffer;
  size_t totWritten;
  ssize_t numWritten;
  for(totWritten=0;totWritten<n;){
    numWritten = write(fd,buf,n-totWritten);
    if(numWritten<=0){
      if(numWritten==-1 && errno==EINTR)
        continue;
      else 
        return -1;
    }
    totWritten += numWritten;
    buf += numWritten;
  }
  return totWritten;
}
