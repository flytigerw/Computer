
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include "tlpi_hdr.h"


int main(int argc,char* argv[]){

  if(argc != 2)
    usageErr("%s file\n",argv[0]);

  int fd = open(argv[1],O_RDONLY);
  if(fd ==-1)
    errExit("open");
  //three buffers
  struct iovec iov[3];
  struct stat myStat;
  int x;
#define  STR_LEN 100
  char str[STR_LEN];
  //set iov with three buffers
  int totRequired=0;

  iov[0].iov_base=&myStat;
  iov[0].iov_len=sizeof(struct stat);
  totRequired += sizeof(struct stat);


  iov[1].iov_base=&x;
  iov[1].iov_len=sizeof(int);
  totRequired += sizeof(int);

  iov[2].iov_base=&str;
  iov[2].iov_len=STR_LEN;
  totRequired += STR_LEN;

  //scatter io 
  int numRead = readv(fd,iov,3);

  if(numRead == -1)
    errExit("read");

  if(numRead < totRequired)
    printf("read fewer bytes than totRequired\n");

  printf("total bytes requested:%ld; bytes read:%ld\n",(long)totRequired,(long)numRead);


  exit(EXIT_SUCCESS);
}
