/**
 * Server (main) header
 */

#ifndef server_h
#define server_h

#include <pthread.h>

#include "serverConfig.h"

typedef struct WorkerArguments_t {
        int iWorkerSocketID;
        char* pcFilePath;
        char* pcFileName;
        pthread_t tid;
        unsigned char ucFinished: 1;
} WorkerArguments;

typedef enum ShutdownState_t {
        noShutdown,
        friendlyShutdown,
        forceShutdown
} ShutdownState;

extern volatile ShutdownState serverShutdownState;

void parseAndConfigure(int argc, char* argv[], ServerConfiguration* psArguments);

void sigintHandler(int iSignum);

#endif
