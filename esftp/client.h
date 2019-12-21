/**
 * @file client.h
 * @brief File contains the header for the client.
 * @author Hannes Braun
 * @date 18.06.2019
 */

#ifndef client_h
#define client_h

#include <arpa/inet.h>

#define RECVBUFFERSIZE 4096

struct Config {
        struct in_addr addr;
        short int port;
        unsigned char printVersion: 1;
};

int parseAndConfigure(int argc, char* argv[], struct Config* config);

int connectAndReceive(struct Config* config);

#endif
