/**
 * Lobby module for the server
 */

#include "lobby.h"

#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "printVersion.h"
#include "server.h"
#include "serverConfig.h"
#include "worker.h"
#include "workerList.h"

int handleRequest(int lobbySocketID, struct WorkerList* workerList, struct LobbyConfig* lobbyConfig);
int checkItemAvaliability(struct LobbyConfig* config);

/**
 * The lobby mainly handles new connections and starts the worker threads.
 * It also includes other smaller functionalies.
 */
int lobby(struct LobbyConfig* config)
{
        // Worker list
        struct WorkerList workerList;

        // General purpose return value
        int retVal = 0;
        int tmp;

        // The socket ids of the lobby and the last created worker
        int lobbySocketID;

        struct sockaddr_in lobbyAddr;

        if (config->printVersion == 1) {
                // Only print the version and exit
                printVersion(server);
                return 0;
        }

        printf("Starting esftp server...\n");

        tmp = checkItemAvaliability(config);
        if (tmp == -1) {
                // Running the server/accessing the items is probably not going to work
                return -1;
        }

        // Initialize worker list
        tmp = wlInitialize(&workerList);
        if (tmp == -1) {
                retVal = -1;
                goto errorWLInit;
        }

        // Create lobby socket
        lobbySocketID = socket(AF_INET, SOCK_STREAM, 0);
        if (lobbySocketID == -1) {
                retVal = -1;
                perror("An error ocurred while creating the lobby socket");
                goto errorSocketBuild;
        }

        // Setting properties for lobby address
        lobbyAddr.sin_family = AF_INET;
        lobbyAddr.sin_port = htons(config->port);
        lobbyAddr.sin_addr.s_addr = INADDR_ANY;

        // Binding
        tmp = bind(lobbySocketID, (struct sockaddr*) &lobbyAddr, sizeof(lobbyAddr));
        if (tmp == -1) {
                retVal = -1;
                perror("An error ocurred while binding");
                goto errorSocketBind;
        }

        // Activating listening mode
        tmp = listen(lobbySocketID, BACKLOGSIZE);
        if (tmp == -1) {
                retVal = -1;
                perror("An error ocurred while setting socket to listening mode");
                goto errorSocketListen;
        }

        // Main loop
        while (serverShutdownState == noShutdown) {
                tmp = handleRequest(lobbySocketID, &workerList, config);
                switch (tmp) {
                        case -1:
                        case -2:
                        case -4:
                                // Try again, but wait a second to avoid wasting cpu time
                                sleep(1);
                                break;
                        case -3:
                        case -5:
                        case -6:
                        case -7:
                                // Stop the server
                                serverShutdownState = forceShutdown;
                                retVal = -1;
                                break;
                        default:
                                break;
                }
        }

        if (serverShutdownState == friendlyShutdown) {
                printf("\x1b[2DInitiating friendly shutdown...\n");
        }

errorSocketListen:
errorSocketBind:

        // Closing the lobby socket
        tmp = close(lobbySocketID);
        if (tmp == -1) {
                retVal = -1;
                perror("An error ocurred while closing the lobby socket");
        }

        // Wait until all workers are finished (incl. forceShutdown: workers will terminate as soon as possible)
        tmp = wlJoin(&workerList);
        if (tmp == -1) {
                retVal = -1;
        }

errorSocketBuild:

        wlFree(&workerList);

errorWLInit:

        return retVal;
}

int handleRequest(int lobbySocketID, struct WorkerList* workerList, struct LobbyConfig* lobbyConfig)
{
        int tmp;
        int retVal = 0;
        int tryAgain;

        // Thread id of the last created worker thread
        pthread_t tidWorker;

        int workerSocketID;
        struct sockaddr_in workerAddr;
        unsigned int workerAddrSize = sizeof(workerAddr);

        // Allocate memory for the worker config
        struct WorkerConfig* workerConfig = (struct WorkerConfig*) malloc(sizeof(struct WorkerConfig));
        if (workerConfig == NULL) {
                fprintf(stderr, "An error ocurred while allocating memory for the worker config. Trying to clean up the worker list and trying again...\n");

                // Not catching return value here because this should an error shouldn't occur at cleanup
                // Also, this seems irrelevant, since an error already ocurred.
                wlCleanup(workerList);
                retVal = -1;

                goto cleanup;
        }

        // Accept the connection
        workerSocketID = accept(lobbySocketID, (struct sockaddr*) &workerAddr, &workerAddrSize);
        if (workerSocketID == -1) {
                switch (errno) {
                        case ECONNABORTED:
                        case EMFILE:
                        case ENFILE:
                        case ENOBUFS:
                                perror("An non-fatal error ocurred while accepting a connection");
                        case EINTR: // Interrupted (likley by SIGINT)
                                // New iteration, try again
                                retVal = -2;
                                break;
                        default:
                                // Error -> leave lobby
                                retVal = -3;
                                perror("An error orucced while accepting a connection");
                                break;
                }
                goto cleanup;
        }

        // Fill worker config
        workerConfig->socketID = workerSocketID;
        workerConfig->items = lobbyConfig->items;
        workerConfig->itemsLen = lobbyConfig->itemsLen;
        workerConfig->finished = 0;

        // Create worker thread
        tmp = pthread_create(&tidWorker, NULL, worker, workerConfig);
        if (tmp != 0) {
                fprintf(stderr, "An error ocurred while starting the worker: %s\n", strerror(tmp));

                if (errno == EAGAIN) {
                        // Not enough resources, try again
                        retVal = -4;
                } else {
                        retVal = -5;
                }

                goto cleanup;
        }

        // Worker list cleanup (also reduces problems when adding the next worker)
        tmp = wlCleanup(workerList);
        if (tmp == -1) {
                // Should hopefully not occur
                retVal = -6;
        }

        // Add created thread/worker to worker list
        workerConfig->tid = tidWorker;
        tmp = wlAdd(workerList, workerConfig);
        if (tmp == -1) {
                // Probably out of memory -> better shut down the server to avoid memory leaks
                retVal = -7;
        }

cleanup:

        if (retVal >= -4 && retVal <= -7) {
                // Close socket since an error ocurred
                tryAgain = 3;
closeInterrupt:
                tmp = close(workerSocketID);
                if (tmp == -1) {
                        if (errno == EINTR) {
                                // Try again
                                tryAgain--;
                                if (tryAgain > 0) {
                                        goto closeInterrupt;
                                }
                        }

                        if (retVal == -4) {
                                // Don't try again, better shut down the server
                                retVal = -5;
                        }
                        perror("An error ocurred while closing the worker socket after pthread_failed");
                }
        }

        if (retVal < -1) {
                free(workerConfig);
        }


        return retVal;
}


int checkItemAvaliability(struct LobbyConfig* config)
{
        int currentItem = 0;
        int retVal = 0;
        int i;

        for (i = 0; i < config->itemsLen; i++) {
                if (access(config->items[currentItem], R_OK) == -1) {
                        // Item doesn't exist or is not readable... it's an error
                        retVal = -1;
                        fprintf(stderr, "Error while trying to access %s: %s\n", config->items[currentItem], strerror(errno));
                }
        }

        return retVal;
}
