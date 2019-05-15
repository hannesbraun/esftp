#ifndef client_h
#define client_h

#include <arpa/inet.h>

#define FALSE 0
#define TRUE 1

typedef struct ClientArguments_t {
    struct in_addr sIPAddress;
    short int siPort;
    unsigned char ucArgumentsValid;
} ClientArguments;

void parseArguments(int argc, char* argv[], ClientArguments* psArguments);

#endif
