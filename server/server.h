/**
 * @file server.h
 * @brief File contains the header for the server.
 * @author Hannes Braun
 * @date 18.06.2019
 */

#ifndef server_h
#define server_h

#include "serverConfig.h"

#define ESFTP_VERSION "ESFTP 0.1.1\n"

#define FALSE 0
#define TRUE 1

typedef struct ServerConfiguration_t
{
    char* pcFilePath;
    short int siPort;
    unsigned char ucVersionFlag: 1;
    unsigned char ucArgumentsValid: 1;
} ServerConfiguration;

typedef struct WorkerArguments_t
{
    int iWorkerSocketID;
    char* pcFilePath;
    char* pcFileName;
} WorkerArguments;

typedef struct ServerControl_t
{
    int iLobbySocketID;
} ServerControl;

void parseAndConfigure(int argc, char* argv[], ServerConfiguration* psArguments);

void lobby(ServerConfiguration* psConfiguration);

void* worker(void* pvArguments);

#endif
