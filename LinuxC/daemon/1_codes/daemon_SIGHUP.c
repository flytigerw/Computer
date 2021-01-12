#include <sys/stat.h>
#include <signal.h>
#include "tlpi_hdr.h"
#include "become_daemon.h"

static const char* LOG_FILE = "/tmp/ds.log";
static const char* CONFIG_FILE = "/tmp/ds/conf";

static volatile sig_atomic_t hupReceived = 0;

static void sighupHandler(int sig){
  hupReceived = 1;
}

int main(int argc,char* argv[]){
  const int SLEEP_TIME = 15;            //Time to sleep between messages
  int count = 0;                       //Number of completed SLEEP_TIME intervals
  int unslept;                         //Time remaining in sleep interval
  //Install handler for SIGHUP
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = sighupHandler;
  if(sigaction(SIGHUP,&sa,NULL) == -1)
    errExit("sigaction");

  if(becomeDaemon(0) == -1)
    errExit("becomeDaemon");


}
