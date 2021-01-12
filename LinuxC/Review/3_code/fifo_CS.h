

//client-server app using fifo

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#define SERVER_FIFO "/tmp/server_fifo"
#define CLIENT_FIFO_TEMPLATE "/tmp/client_fifo_%ld"   //用client pid填充

#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE) + 20)

//request message
struct request{
    pid_t pid;
    int msg_len;  //模拟消息长度
};

//response message 
struct response{
    int msg_seq;  //sever 向 client 反馈client发送的message在整个message序列中所处的位置
};
