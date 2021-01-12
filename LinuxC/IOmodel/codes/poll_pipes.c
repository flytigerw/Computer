

#include <time.h>
#include <poll.h>
#include "tlpi_hdr.h"

int main(int argc,char* argv[]){
  if(argc < 2)
    usageErr("%s num-pipes [num-writes]\n",argv[0]);

  //allocate arrayr we want to use 
  int numPipes = getInt(argv[1],GN_GT_0,"numPipes");
  int (*pfds)[2] =(int(*)[2]) calloc(numPipes,sizeof(int[2]));
  if(pfds == NULL)
    errExit("calloc");
  struct pollfd* pollFds;
  pollFds = (struct pollfd*)calloc(numPipes,sizeof(struct pollfd));
  if(pollFds == NULL)
    errExit("calloc");

  //create the pipes 
  for(int i=0;i<numPipes;i++)
    if(pipe(pfds[i]) == -1)
      errExit("pipe %d",i);

  //perform spcified numbef of wirtes to random pipes 
  int numWrites = (argc > 2) ? getInt(argv[2],GN_GT_0,"num-writes") : 1;
  srandom((int)time(NULL));
  int randPipe;
  for(int i=0;i<numWrites;i++){
      randPipe = random() % numPipes;
      printf("Writing to fd:%3d(read fd:%3d)\n",pfds[randPipe][1],pfds[randPipe][0]);
      if(write(pfds[randPipe][1],"a",1) == -1)
        errExit("write %d",pfds[randPipe][1]);
  }

  //Build the fd list suppiled to poll(). It is set to contain the fds for read ends of all of the pipes 
  for(int i=0; i<numPipes;i++){
    pollFds[i].fd = pfds[i][0];
    pollFds[i].events = POLLIN;
  }

  int ready = poll(pollFds,numPipes,-1);
  if(ready == -1)
    errExit("poll");
  printf("poll() returned %d\n",ready);

  //chech which pipes have data avaliable for reading 
  for(int i=0;i<numPipes;i++)
    if(pollFds[i].revents & POLLIN)
      printf("Readable: %d %3d\n",i,pollFds[i].fd);

  exit(EXIT_SUCCESS);
}

