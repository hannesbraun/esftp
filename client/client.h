/**
 * @file client.h
 * @brief File contains the header for the client.
 * @author Hannes Braun
 * @date 18.06.2019
 */

#ifndef client_h
#define client_h

#include <arpa/inet.h>

#define FALSE 0
#define TRUE 1

#define RECVBUFFERSIZE 4096

typedef struct ClientArguments_t
{
    struct in_addr sIPAddress;
    short int siPort;
    unsigned char ucArgumentsValid;
} ClientArguments;

void parseArguments(int argc, char* argv[], ClientArguments* psArguments);

void connectAndReceive(ClientArguments* psArguments);

#endif
