#include <sys/un.h>
#include <sys/socket.h>
#include <ctype.h>
#include "tlpi_hdr.h"

#define BUF_SIZE 10  //Maximum of messages of exchanged between client and server 
#define SV_SOCK_PAHT "/tmp/ud_ucase"
