#ifndef server_h
#define server_h

#include "serverConfig.h"

#define FALSE 0
#define TRUE 1

typedef struct ServerArguments_t {
    unsigned char ucArgumentsValid;
    short int siPort;
    char* pcFilePath;
} ServerArguments;

typedef struct WorkerArguments_t {
    int iWorkerSocketID;
    char* pcFilePath;
    char* pcFileName;
} WorkerArguments;

typedef struct ServerControl_t {
    int iLobbySocketID;
} ServerControl;

void parseArguments(int argc, char* argv[], ServerArguments* psArguments);

void* lobby(void* pvArguments);

void* worker(void* pvArguments);

#endif
