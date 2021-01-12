#ifndef  INET_SOCKETS_H
#define  INET_SOCKETS_H 

#include <sys/socket.h>
#include <netdb.h>

//create a new socket with the given socket type and connet it to the address specified by host and service 
//designed for clients that need to connect their sockets to a server socket 
//return a fd refer to the new socket 
int inetConnect(const char* host,const char* service, int type);

//create a listening stream socket bound to the wildcard IP address on the TCP port specified by serice 
//designed for use by TCP servers
//return socket fd 
int inetListen(const char* service,int backlog,socklen_t* addrlen);

//create a socket of given type,bound to the wildcard IP address on the port specified by service and type 
//designed primarily for UDP servers and clients to create a socket bound to specific address 
//return new socket fd
int inetBind(const char* service,int type,socklen_t* addrlen);

//convert Internet socket address to printable form
char* inetAddressStr(const struct sockaddr* addr,socklen_t addrlen,char *addrStr,int addrStrlen);

#define  IS_ADDR_STR_LEN 4096 //Suggested length for string buffer that caller should pass to inetAddressStr() Must be greater than (NI_MAXHOST+NI_MAXSERV+$)

#endif
