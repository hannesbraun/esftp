/**
 * @file server.h
 * @brief File contains the header for the server.
 * @author Hannes Braun
 * @date 18.06.2019
 */

#ifndef server_h
#define server_h

#include <pthread.h>

#include "serverConfig.h"

#define FALSE 0
#define TRUE 1

typedef struct ServerConfiguration_t {
        char* pcFilePath;
        short int siPort;
        unsigned char ucVersionFlag: 1;
        unsigned char ucArgumentsValid: 1;
} ServerConfiguration;

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

void lobby(ServerConfiguration* psConfiguration);

void* worker(void* pvArguments);

void sigintHandler(int iSignum);

#endif
