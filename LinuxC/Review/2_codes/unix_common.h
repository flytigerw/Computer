

// client-server program in UNIX domain
// client从stdin读取数据，通过socket发送到server，server在将数据通过stdout输出

#include <sys/un.h>
#include <sys/socket.h>
#include "tlpi_hdr.h"

//server socket address 
#define  PATHNAME "/tmp/unix_sock"
#define BUFSIZE  100
