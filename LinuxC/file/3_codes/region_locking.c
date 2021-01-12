
#include <fcntl.h>
#include "region_locking.h"

//Lock a file region 
static int 
lockReg(int fd,int cmd,int type,int whence,int start,off_t len){
  struct flock fl;
  fl.l_type = type;
  fl.l_whence = whence;
  fl.l_start = start;
  fl.l_len = len;
  return  fcntl(fd,cmd,&fl);
}

//Lock a file region using nonblocking F_SETLK 
int lockRegion(int fd,int type,int whence,int start,int len){
  return lockReg(fd,F_SETLK,type,whence,start,len);
}

//Lock a file region using blocking F_SETLKW 
int lockRegionWair(int fd, int type,int whence,int start,int len){
  return lockReg(fd,F_SETLKW,type,whence,start,len);
}

//Test if a file region is lockable.Return 0 --> lockable or PID of process holding incompatible lock or -1 on error
pid_t regionIsLocked(int fd,int type,int whenece,int start,int len){
  struct flock fl;
  fl.l_type = type;
  fl.l_whence = whenece;
  fl.l_start = start;
  fl.l_len = len;
  if(fcntl(fd,F_GETLK,&fl) == -1)
    return -1;
  return (fl.l_type == F_UNLCK) ? 0 : fl.l_pid;
}
