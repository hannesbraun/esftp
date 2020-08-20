/******************************************************************************
 * Copyright 2019-2020 Hannes Braun
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 ******************************************************************************/

/**
 * Lobby module for the server
 */

#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "lobby.h"
#include "server.h"
#include "serverConfig.h"
#include "worker.h"
#include "workerList.h"

int handleRequest(int lobbySocketID, struct WorkerList* workerList, struct LobbyConfig* lobbyConfig);
int checkItemAvaliability(struct LobbyConfig* config);

/**
 * The lobby mainly handles new connections and starts the worker threads.
 */
int lobby(struct LobbyConfig* config)
{
        // Worker list
        struct WorkerList workerList;

        // General purpose return variable
        int tmp;

        // Function return value
        int retVal = 0;

        // The socket id of the lobby
        int lobbySocketID;

        // Lobby address
        struct sockaddr_in lobbyAddr;

        printf("Starting esftp server...\n");

        tmp = checkItemAvaliability(config);
        if (tmp == -1) {
                // Not all items are available
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
                perror("An error ocurred while binding the lobby socket");
                goto errorSocketBind;
        }

        // Activating listening mode
        tmp = listen(lobbySocketID, BACKLOGSIZE);
        if (tmp == -1) {
                retVal = -1;
                perror("An error ocurred while setting the lobby socket to listening mode");
                goto errorSocketListen;
        }

        // Main loop
        while (serverShutdownState == noShutdown) {
                tmp = handleRequest(lobbySocketID, &workerList, config);
                switch (tmp) {
                        case -1:
                        // Probably not enough free memory available to allocate space for the worker config
                        // Worker list has been cleaned up
                        case -2:
                        // Non-fatal error while accepting the connection (including interruption by SIGINT)
                        case -4:
                        // Not enough resources to start the worker thread
                        case -7:
                                // Adding worker config to worker list was not possible

                                // Try again, but wait a second to avoid wasting cpu time
                                sleep(1);
                                break;

                        // Reasons to leave the lobby
                        case -3:
                        // Error while accepting a connection
                        case -5:
                        // pthread_create error that shouldn't occur or
                        case -6:
                        // Error while cleaning up worker list
                        // Should never happen
                        // I believe it's pretty critical if it does happen anyway, so a shutdown might be a good idea.
                        case -8:
                        // Unable to close the socket during error handling
                        // Too much interruptions
                        case -9:
                                // Unable to close the socket during error handling

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

/**
 * Handles an incoming request and starts a worker.
 */
int handleRequest(int lobbySocketID, struct WorkerList* workerList, struct LobbyConfig* lobbyConfig)
{
        // General purpose return variable
        int tmp;

        // Function return value
        int retVal = 0;

        // Original return value (before an error occurs in the error handling part)
        int origRetVal;

        // Counting how often to try again closing the socket while handling an error
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

                // Not catching return value here because an error shouldn't occur at cleanup
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
                        // fallthrough
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
        workerConfig->selfFree = 0;

        // Create worker thread
        tmp = pthread_create(&tidWorker, NULL, worker, workerConfig);
        if (tmp != 0) {
                fprintf(stderr, "An error ocurred while starting the worker thread: %s\n", strerror(tmp));

                if (errno == EAGAIN) {
                        // Not enough resources, try again
                        retVal = -4;
                } else {
                        // Another error, probably not happening
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
                // Probably out of memory/resizing the worker list was not possible
                retVal = -7;
        }

cleanup:
        origRetVal = retVal;
        if (retVal >= -4 && retVal <= -7) {
                // Close socket since an error ocurred
                tryAgain = 10;
closeInterrupt:
                tmp = close(workerSocketID);
                if (tmp == -1) {
                        if (errno == EINTR) {
                                tryAgain--;
                                if (tryAgain > 0) {
                                        // Try to close the socket again
                                        goto closeInterrupt;
                                } else {
                                        fprintf(stderr, "Too much interruptions\n");
                                        retVal = -8;
                                }
                        } else {
                                // Other error while closing the socket (not interrupted)
                                perror("An error ocurred while closing the worker socket for error handling purposes");

                                if (retVal == -4 || retVal == -7) {
                                        // Don't try again, better shut down the server
                                        retVal = -9;
                                }
                        }
                } else {
                        // Invalidate socket for worker
                        workerConfig->socketID = -1;
                }
        }

        // Is memory for the config allocated?
        // And only free allocated memory if worker is not already running (else: the worker has to free the memory)
        if (retVal < -1 && origRetVal != -6 && origRetVal != -7) {
                free(workerConfig);
        } else if (origRetVal == -6 || origRetVal == -7) {
                // Tell worker to free memory himself
                workerConfig->selfFree = 1;
        }

        return retVal;
}

/**
 * Checks if all items are available
 */
int checkItemAvaliability(struct LobbyConfig* config)
{
        // Function return value
        int retVal = 0;

        unsigned long int i;
        for (i = 0; i < config->itemsLen; i++) {
                if (access(config->items[i], R_OK) == -1) {
                        // Item doesn't exist or is not readable... it's an error
                        retVal = -1;
                        fprintf(stderr, "Error while trying to access %s: %s\n", config->items[i], strerror(errno));
                }
        }

        return retVal;
}
