#include <sys/types.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <stddef.h>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/wait.h>
#include "tlpi_hdr.h"

#define SERVER_KEY 0x1aaaaaa1     //key for server's message queue 
 
struct requestMsg{
  long mtype;                       //requests
  int clientId;                     //ID of client's message queue
  char pathname[PATH_MAX];                //File to be returned
};

  //It computes size of 'mtext' part of 'requestMsg' 
  //We use offsefof() to hanlde the possiblity that there are padding bytes between the clientId and pathanme fields

#define REQ_MSG_SIZE (offsetof(struct requestMsg,pathname) - \
                      offsetof(struct requestMsg,clientId) + PATH_MAX)
#define RESP_MSG_SIZE 8192

struct responseMsg{
  long mtype;
  char data[RESP_MSG_SIZE];                //File content
};

//types for response messages
#define RESP_MT_FAILURE 1               //File couldn't be opened
#define RESP_MT_DATA 2                  //Message contains file data 
#define RESP_MT_END  3                 //File data complete
