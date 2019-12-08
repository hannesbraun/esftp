/**
 * Lobby module for the server
 */

#include "lobby.h"

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

int checkItemAvaliability(char** items);

/**
 * The lobby mainly handles new connections and starts the worker threads.
 * It also includes other smaller functionalies.
 */
void lobby(LobbyConfig* config)
{
        // Helping variable to run the basename function on
        //char* pcFileNameAddress;

        // File name to transfer
        //char* pcFileName;

        // Thread id of the last created worker thread
        pthread_t tidWorker;

        // Worker list
        struct WorkerList workerList;

        // General purpose return value
        int retVal;

        // The socket ids of the lobby and the last created worker
        int lobbySocketID;
        int workerSocketID;

        struct sockaddr_in lobbyAddr;

        struct WorkerConfig* workerConfig;

        // Allocate memory for file name
        //pcFileNameAddress = (char*) malloc(sizeof(char) * (strlen(psConfiguration->pcFilePath) + 1));
        //if (pcFileNameAddress == NULL) {
        //        fprintf(stderr, "An error ocurred while allocating memory for the file name.\n");
        //        goto errorDuringMalloc1;
        //}

        // Copy file path and get basename (file name without directories)
        //strncpy(pcFileNameAddress, psConfiguration->pcFilePath, strlen(psConfiguration->pcFilePath) + 1);
        //pcFileName = basename(pcFileNameAddress);

        // Initialize worker list
        retVal = wlInitialize(&workerList);
        if (retVal == -1) {
                fprintf(stderr, "An error ocurred while allocating memory for the worker list.\n");
                goto errorDuringWLInitialization;
        }

        // Create lobby socket
        lobbySocketID = socket(AF_INET, SOCK_STREAM, 0);
        if (lobbySocketID == -1) {
                perror("An error ocurred while creating the lobby socket");
                goto errorDuringSocketBuilding;
        }

        // Setting properties for lobby address
        lobbyAddr.sin_family = AF_INET;
        lobbyAddr.sin_port = htons(config->port);
        lobbyAddr.sin_addr.s_addr = INADDR_ANY;

        // Binding
        retVal = bind(lobbySocketID, (struct sockaddr*) &lobbyAddress, sizeof(lobbyAddress));
        if (retVal == -1) {
                perror("An error ocurred while binding");
                goto errorDuringWork;
        }

        // Activating listening mode
        retVal = listen(lobbySocketID, BACKLOGSIZE);
        if (retVal == -1) {
                perror("An error ocurred while setting socket to listening mode");
                goto errorDuringWork;
        }

        while (serverShutdownState == noShutdown) {
                retVal = handleRequest(lobbySocketID, workerList);
                switch (retVal) {
                        case -1:
                        case -2:
                                sleep(1);
                                break;

                        default:
                                break;
                }
        }

errorDuringWork:

        // Closing the lobby socket
        retVal = close(lobbySocketID);
        if (retVal == -1)
                perror("An error ocurred while closing the lobby socket");

        retVal = wlJoin(&workerList);
        if (retVal == -1)
                perror("An error ocurred while joining the threads");

errorDuringSocketBuilding:

        wlFree(&workerList);

errorDuringWLInitialization:

        free(fileNameAddress);

errorDuringMalloc1:

        return;
}

int handleRequest(int lobbySocketID, WorkerList* workerList)
{
        int tmp;
        int retVal = 0;

        int workerSocketID;
        struct sockaddr_in workerAddr;
        unsigned int workerAddrSize = sizeof(workerAddr);

        // Allocate memory for the worker arguments
        WorkerConfig* workerConfig = (WorkerConfig*) malloc(sizeof(struct WorkerConfig));
        if (workerConfig == NULL) {
                fprintf(stderr, "An error ocurred while allocating memory for the worker arguments. Waiting for the last started worker to finish...\n");

                tmp = wlCleanup(&workerList);
                if (tmp == -1)
                        // Cleanup failed
                        retVal = -2;
                else
                        // No memory, try again
                        retVal = -1;

                goto cleanup;
        }

        // Accept the connection
        workerSocketID = accept(lobbySocketID, (struct sockaddr*) &workerAddr, &workerAddrSize);
        if (workerSocketID == -1) {
                if (errno == EINTR) {
                        // Interrupted (likley by SIGINT), new iteration
                        retVal = -3;
                } else {
                        retVal = -4;
                        perror("An error orucced while accepting a connection");
                }
                goto cleanup;
        }

        // Fill worker config
        workerConfig->socketID = workerSocketID;
        workerConfig->pcFilePath = config->pcFilePath;
        workerConfig->pcFileName = pcFileName;
        workerConfig->finished = 0;

        // Create worker thread
        tmp = pthread_create(&tidWorker, NULL, worker, workerConfig);
        if (tmp != 0) {
                fprintf(stderr, "An error ocurred while starting the worker. pthread_create returned %d\n", tmp);

                tmp = close(workerSocketID);
                if (tmp == -1)
                        retVal = -5;
                perror("An error ocurred while closing the socket in the error handling");
                else
                        retVal = -6;

                goto cleanup;
        }

        // Worker list cleanup (also reduces problems when adding the new worker (next step))
        wlCleanup(&workerList);

        // Add created thread/worker to worker list
        workerConfig->tid = tidWorker;
        tmp = wlAdd(&workerList, workerConfig);
        if (tmp == -1)
                fprintf(stderr, "An error ocurred while adding the thread id to the worker list. This worker will not be tracked.\n");

cleanup:
        if (retVal < 0 && (retVal != -1 && retVal != -2))
                free(workerConfig);


        return retVal;
}


int checkItemAvaliability(char** items)
{
        int currentItem = 0;
        int reVal = 0;

        while (config->items[currentItem] != NULL) {
                if (access(config->items[currentItem], R_OK) == -1) {
                        // File doesn't exist or is not readable
                        retVal = 1;
                        fprintf(stderr, "Error while trying to access %s: %s", config->items[currentItem], strerror(errno));
                }
        }

        return retVal;
}
