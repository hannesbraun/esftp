#ifndef esftpClient_h
#define esftpClient_h

#include <arpa/inet.h>

#define RECVBUFFERSIZE 4096

struct ClientConfig {
        struct in_addr addr;
        short int port;
        unsigned char printVersion: 1;
};

int connectAndReceive(struct ClientConfig* config);

#endif
