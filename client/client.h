/**
 * @file client.h
 * @brief File contains the header for the client.
 * @author Hannes Braun
 * @date 18.06.2019
 */

#ifndef client_h
#define client_h

#include <arpa/inet.h>

#define ESFTP_VERSION "ESFTP 0.1.1\n"
#define ESFTP_PORT 6719

#define FALSE 0
#define TRUE 1

#define RECVBUFFERSIZE 4096

typedef struct ClientConfiguration_t
{
    struct in_addr sIPAddress;
    char* pcOutputFileName;
    short int siPort;
    unsigned char ucVersionFlag: 1;
    unsigned char ucArgumentsValid: 1;
} ClientConfiguration;

void parseAndConfigure(int argc, char* argv[], ClientConfiguration* psConfiguration);

void connectAndReceive(ClientConfiguration* psConfiguration);

void printStatus(char* const pcStatusString, short int siPreviousStrLen);

#endif
