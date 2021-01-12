

// client-server using UNIX domain socket sockets
// client transfers the CMD line argument to server and the server converted the received text to upper case and send them back to client ---> so the server must know the address the client

#include <sys/un.h>
#include <sys/socket.h>
#include "tlpi_hdr.h"

#define PATHNAME "/tmp/my_socket"
#define BUFSIZE 100
