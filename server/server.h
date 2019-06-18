/**
 * @file server.h
 * @brief File contains the header for the server.
 * @author Hannes Braun
 * @date 18.06.2019
 */

#ifndef server_h
#define server_h

#include "serverConfig.h"

#define FALSE 0
#define TRUE 1

typedef struct ServerArguments_t
{
    char* pcFilePath;
    short int siPort;
    unsigned char ucArgumentsValid;
} ServerArguments;

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

void parseArguments(int argc, char* argv[], ServerArguments* psArguments);

void* lobby(void* pvArguments);

void* worker(void* pvArguments);

#endif
