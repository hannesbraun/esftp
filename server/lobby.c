/**
 * @file lobby.c
 * @brief File contains the lobby function.
 * @author Hannes Braun
 * @date 12.05.2019
 */

#include <arpa/inet.h>
#include <libgen.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "server.h"

/**
 * @fn void* lobby(void* pvArguments)
 * @brief Handles new connections and starts the worker threads.
 * @param pvArguments the arguments to run this lobby with (provided as a ServerArguments struct)
 * @return void* This function is just returning NULL to satisfy pthread.
 * @author Hannes Braun
 * @date 07.06.2019
 */
void* lobby(void* pvConfiguration)
{
    // Casting argument for better usage
    ServerConfiguration* psConfiguration = (ServerConfiguration*) pvConfiguration;

    // Helping variable to run the basename function on
    char* pcFileNameAddress;

    // File name to transfer
    char* pcFileName;

    // Thread id of the last created worker thread
    pthread_t tidWorkerThread;

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
    if (pcFileNameAddress == NULL)
    {
        fprintf(stderr, "An error ocurred while allocating memory for the file name.");
    }

    // Copy file path and get basename (file name without directories)
    strncpy(pcFileNameAddress, psConfiguration->pcFilePath, strlen(psConfiguration->pcFilePath) + 1);
    pcFileName = basename(pcFileNameAddress);

    // Create lobby socket
    iLobbySocketID = socket(AF_INET, SOCK_STREAM, 0);
    if (iLobbySocketID == -1)
    {
        perror("An error ocurred while creating the lobby socket");
    }

    // Setting properties for lobby address
    sLobbyAddress.sin_family = AF_INET;
    sLobbyAddress.sin_port = htons(psConfiguration->siPort);
    sLobbyAddress.sin_addr.s_addr = INADDR_ANY;

    // Binding
    iReturnValue = bind(iLobbySocketID, (struct sockaddr*) &sLobbyAddress, sizeof(sLobbyAddress));
    if (iReturnValue == -1)
    {
        perror("An error ocurred while binding");
    }

    // Activating listening mode
    iReturnValue = listen(iLobbySocketID, BACKLOGSIZE);
    if (iReturnValue == -1)
    {
        perror("An error ocurred while setting socket to listening mode");
    }

    while (1)
    {
        // Allocate memory for the worker arguments
        psWorkerArguments = (WorkerArguments*) malloc(sizeof(WorkerArguments));
        if (psWorkerArguments == NULL)
        {
            fprintf(stderr, "An error ocurred while allocating memory for the worker arguments.");
        }

        // Accept the connection
        iWorkerSocketID = accept(iLobbySocketID, (struct sockaddr*) &sWorkerAddress, &uiWorkerAddressSize);
        if (iWorkerSocketID == -1)
        {
            perror("Error while accepting connection");
        }

        // Fill worker arguments
        psWorkerArguments->iWorkerSocketID = iWorkerSocketID;
        psWorkerArguments->pcFilePath = psConfiguration->pcFilePath;
        psWorkerArguments->pcFileName = pcFileName;

        // Create worker thread
        iReturnValue = pthread_create(&tidWorkerThread, NULL, worker, psWorkerArguments);
        if (iReturnValue != 0)
        {
            fprintf(stderr, "An error ocurred while starting the worker. pthread_create returned %d", iReturnValue);
        }
    }

    // Closing the lobby socket
    iReturnValue = close(iLobbySocketID);
    if (iReturnValue == -1)
    {
        perror("An error ocurred while closing the lobby socket");
    }

    free(pcFileNameAddress);

    return NULL;
}
