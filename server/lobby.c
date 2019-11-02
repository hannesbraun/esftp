/**
 * @file lobby.c
 * @brief File contains the lobby function.
 * @author Hannes Braun
 * @date 12.05.2019
 */

#include <arpa/inet.h>
#include <errno.h>
#include <libgen.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "workerList.h"
#include "server.h"

/**
 * @fn void lobby(ServerConfiguration* psConfiguration)
 * @brief Handles new connections and starts the worker threads.
 * @param psConfiguration the arguments to run this lobby with
 * @return void
 * @author Hannes Braun
 * @date 21.07.2019
 */
void lobby(ServerConfiguration* psConfiguration)
{
        // Helping variable to run the basename function on
        char* pcFileNameAddress;

        // File name to transfer
        char* pcFileName;

        // Thread id of the last created worker thread
        pthread_t tidWorkerThread;

        // Worker list
        WorkerList workerList;

        // General purpose return value
        int iReturnValue;

        // The socket ids of the lobby and the last created worker
        int iLobbySocketID;
        int iWorkerSocketID;

        struct sockaddr_in sLobbyAddress;
        struct sockaddr_in sWorkerAddress;

        WorkerArguments* psWorkerArguments;

        unsigned int uiWorkerAddressSize = sizeof(struct sockaddr_in);

        // Allocate memory for file name
        pcFileNameAddress = (char*) malloc(sizeof(char) * (strlen(psConfiguration->pcFilePath) + 1));
        if (pcFileNameAddress == NULL) {
                fprintf(stderr, "An error ocurred while allocating memory for the file name.\n");
                goto errorDuringMalloc1;
        }

        // Copy file path and get basename (file name without directories)
        strncpy(pcFileNameAddress, psConfiguration->pcFilePath, strlen(psConfiguration->pcFilePath) + 1);
        pcFileName = basename(pcFileNameAddress);

        // Initialize worker list
        iReturnValue = wlInitialize(&workerList);
        if (iReturnValue == -1) {
                fprintf(stderr, "An error ocurred while allocating memory for the worker list.\n");
                goto errorDuringWLInitialization;
        }

        // Create lobby socket
        iLobbySocketID = socket(AF_INET, SOCK_STREAM, 0);
        if (iLobbySocketID == -1) {
                perror("An error ocurred while creating the lobby socket");
                goto errorDuringSocketBuilding;
        }

        // Setting properties for lobby address
        sLobbyAddress.sin_family = AF_INET;
        sLobbyAddress.sin_port = htons(psConfiguration->siPort);
        sLobbyAddress.sin_addr.s_addr = INADDR_ANY;

        // Binding
        iReturnValue = bind(iLobbySocketID, (struct sockaddr*) &sLobbyAddress, sizeof(sLobbyAddress));
        if (iReturnValue == -1) {
                perror("An error ocurred while binding");
                goto errorDuringWork;
        }

        // Activating listening mode
        iReturnValue = listen(iLobbySocketID, BACKLOGSIZE);
        if (iReturnValue == -1) {
                perror("An error ocurred while setting socket to listening mode");
                goto errorDuringWork;
        }

        while (serverShutdownState == noShutdown) {
                // Allocate memory for the worker arguments
                psWorkerArguments = (WorkerArguments*) malloc(sizeof(WorkerArguments));
                if (psWorkerArguments == NULL) {
                        fprintf(stderr, "An error ocurred while allocating memory for the worker arguments. Waiting for the last started worker to finish...\n");

                        // Sleep one second to avoid wasting cpu time in case of looping
                        sleep(1);

                        iReturnValue = wlCleanup(&workerList);
                        if (iReturnValue == -1)
                                goto errorDuringWork;

                        continue;
                }

                // Accept the connection
                iWorkerSocketID = accept(iLobbySocketID, (struct sockaddr*) &sWorkerAddress, &uiWorkerAddressSize);
                if (iWorkerSocketID == -1) {
                        free(psWorkerArguments);

                        if (errno == EINTR) {
                                // Interrupted (likley by SIGINT)
                                continue;
                        } else {
                                perror("An error orucced while accepting a connection");
                                goto errorDuringWork;
                        }
                }

                // Fill worker arguments
                psWorkerArguments->iWorkerSocketID = iWorkerSocketID;
                psWorkerArguments->pcFilePath = psConfiguration->pcFilePath;
                psWorkerArguments->pcFileName = pcFileName;
                psWorkerArguments->ucFinished = 0;

                // Create worker thread
                iReturnValue = pthread_create(&tidWorkerThread, NULL, worker, psWorkerArguments);
                if (iReturnValue != 0) {
                        fprintf(stderr, "An error ocurred while starting the worker. pthread_create returned %d\n", iReturnValue);
                        free(psWorkerArguments);

                        iReturnValue = close(iWorkerSocketID);
                        if (iReturnValue == -1)
                                perror("An error ocurred while closing the socket in the error handling");

                        continue;
                }

                // Worker list cleanup (also reduces problems when adding the new worker (next step))
                wlCleanup(&workerList);

                // Add created thread/worker to worker list
                psWorkerArguments->tid = tidWorkerThread;
                iReturnValue = wlAdd(&workerList, psWorkerArguments);
                if (iReturnValue == -1)
                        fprintf(stderr, "An error ocurred while addint the thread id to the worker list. This worker will not be tracked.\n");
        }

errorDuringWork:

        // Closing the lobby socket
        iReturnValue = close(iLobbySocketID);
        if (iReturnValue == -1)
                perror("An error ocurred while closing the lobby socket");

        iReturnValue = wlJoin(&workerList);
        if (iReturnValue == -1)
                perror("An error ocurred while joining the threads");

errorDuringSocketBuilding:

        wlFree(&workerList);

errorDuringWLInitialization:

        free(pcFileNameAddress);

errorDuringMalloc1:

        return;
}
