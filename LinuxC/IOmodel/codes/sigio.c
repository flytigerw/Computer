#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <termios.h>
#include "tlpi_hdr.h"
#include "tty_functions.h"
static volatile sig_atomic_t gotSigio = 0;

static void sigioHandler(int sig){
  gotSigio = 1;
}

int main(int argc,char* argv[]){

  //Establish handler 
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = sigioHandler;
  if(sigaction(SIGIO,&sa,NULL) == -1)
    errExit("sigaction");

  //set owner --> the process itself 
  if(fcntl(STDIN_FILENO,F_SETOWN,getpid()) == -1)
    errExit("fcntl(F_SETOWN)");

  //set O_ASNYC and O_NONBLOCK flag 
  int flags = fcntl(STDIN_FILENO,F_GETFL);
  if(fcntl(STDIN_FILENO,F_SETFL,flags | O_ASYNC | O_NONBLOCK) == -1)
    errExit("fcntl");
  
  //place terminal in cbreak mode
  struct termios origTermios;
  if(ttySetCbreak(STDIN_FILENO,&origTermios) == -1)
    errExit("ttySetCbreak");

  Boolean done;
  int cnt;
  char c;
  for(done = FALSE,cnt = 0; !done; cnt++){
    for(int i=0;i<10000000;i++)        //slow main loop down a little
      continue;
    if(gotSigio){    //Is input avaliable
      //read all input until an error or EOF or a hash(#) character is read;
      
      while(read(STDIN_FILENO,&c,1) > 0 && !done){
        printf("cnt = %d;read %c\n",cnt,c);
        if(c == '#')
          done = TRUE;
      }
      gotSigio = 0;
    }
  }
  if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&origTermios) == -1)
    errExit("tcsetattr");
  exit(EXIT_SUCCESS);
}
