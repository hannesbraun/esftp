#ifndef client_h
#define client_h

#include <arpa/inet.h>

#define RECVBUFFERSIZE 4096

struct ClientConfig {
        struct in_addr addr;
        short int port;
        unsigned char printVersion: 1;
};

int parseAndConfigure(int argc, char* argv[], struct ClientConfig* config);

int connectAndReceive(struct ClientConfig* config);

#endif
