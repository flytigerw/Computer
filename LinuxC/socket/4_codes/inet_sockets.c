#define _BSD_SOURCE  //to get NI_MAXHOST and NI_MAXSERV definations 
#include<sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "inet_sockets.h"
#include "tlpi_hdr.h"


int inetConnect(const char* host,const char* service,int type){
    struct addrinfo hints;
    struct addrinfo* result,*tmp;
    memset(&hints,0,sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family = AF_UNSPEC;   ///both for IPv4 and IPv6
    hints.ai_socktype = type;

    int s = getaddrinfo(host,service,&hints,&result);
    if(s != 0){
      errno = ENOSYS;
      return -1;
    }
    
    //walk through the return list 
    int sfd;
    for(tmp = result; tmp != NULL; tmp=tmp->ai_next){
      sfd = socket(tmp->ai_family,tmp->ai_socktype,tmp->ai_protocol);
      if(sfd == -1)
        continue;    //On error and try next address 
      
      if(connect(sfd,tmp->ai_addr,tmp->ai_addrlen) != -1)
        break;  //success

      //connect failed: close this socket and try next address 
      close(sfd);
    }
    freeaddrinfo(result);
    return (tmp==NULL) ? -1 : sfd;
}

//Public interface:inetBind() and inetListen()
static int inetPassiveSocket(const char* service,int type,socklen_t* addrlen,Boolean doListen,int backLog){
     struct addrinfo hints;
     struct addrinfo *result,*tmp;
     int sfd,optval,s;
     memset(&hints,0,sizeof(struct addrinfo));
     hints.ai_canonname = NULL;
     hints.ai_addr = NULL;
     hints.ai_next = NULL;
     hints.ai_socktype = type;
     hints.ai_family = AF_UNSPEC;
     hints.ai_flags = AI_PASSIVE;  // Use wildcard IP address 

     if((s=getaddrinfo(NULL,service,&hints,&result)) != 0)
       return -1;

     //walk through the returned list 
     optval = 1;
     for(tmp = result; tmp != NULL; tmp=tmp->ai_next){
       sfd = socket(tmp->ai_family,tmp->ai_socktype,tmp->ai_protocol);
       if(sfd == -1)
         continue;
       if(doListen){
         if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval)) == -1){
           close(sfd);
           freeaddrinfo(result);
           return -1;
         }
       }
       if(bind(sfd,tmp->ai_addr,tmp->ai_addrlen) == 0)
         break;

       //bind() failed:close this socket and try next address 
       close(sfd);
     }
     if(tmp != NULL && doListen){
       if(listen(sfd,backLog) == -1){
         freeaddrinfo(result);
         return -1;
       }
     }

    if(tmp != NULL && addrlen != NULL)
      *addrlen = tmp->ai_addrlen;  //return address structure size 
    freeaddrinfo(result);
    return(tmp==NULL) ? -1 : sfd;
}

int inetListen(const char* service, int backLog, socklen_t* addrlen){
  return inetPassiveSocket(service,SOCK_STREAM,addrlen,TRUE,backLog);
}
int inetBind(const char* service, int type, socklen_t* addrlen){
  return inetPassiveSocket(service,type,addrlen,FALSE,0);
}

char * inetAddressStr(const struct sockaddr* addr,socklen_t addrlen, char* addrStr,int addrStrLen){
  char host[NI_MAXHOST], service[NI_MAXSERV];
  if(getnameinfo(addr,addrlen,host,NI_MAXHOST,service,NI_MAXSERV,NI_NUMERICSERV) == 0)
    snprintf(addrStr,addrStrLen,"(%s,%s)",host,service);
  else 
    snprintf(addrStr,addrStrLen,"(?UNKNOWN?)");

  addrStr[addrStrLen-1]='\0';   //Ensure result is null-terminated
  return addrStr;
}
