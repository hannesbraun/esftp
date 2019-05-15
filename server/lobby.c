/**
 * @file main.c
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

#include "server.h"

void* lobby(void* pvArguments)
{
    // Casting argument for better usage
    ServerArguments* psArguments = (ServerArguments*) pvArguments;
    
    char* pcFileNameAddress;
    char* pcFileName;
    
    pthread_t tidWorkerThread;
    
    // General purpose return value
    int iReturnValue;
    
    int iLobbySocketID;
    int iWorkerSocketID;
    
    struct sockaddr_in sLobbyAddress;
    struct sockaddr_in sWorkerAddress;
    
    WorkerArguments* psWorkerArguments;
    
    unsigned int uiWorkerAddressSize = sizeof(struct sockaddr_in);
    
    // Allocate memory for file name
    pcFileNameAddress = (char*) malloc(sizeof(char) * (strlen(psArguments->pcFilePath) + 1));
    if (pcFileNameAddress == NULL)
    {
        fprintf(stderr, "An error ocurred while allocating memory for the file name.");
    }
    
    // Copy file path and get basename
    strncpy(pcFileNameAddress, psArguments->pcFilePath, strlen(psArguments->pcFilePath) + 1);
    pcFileName = basename(pcFileNameAddress);
    
    iLobbySocketID = socket(AF_INET, SOCK_STREAM, 0);
    if (iLobbySocketID == -1)
    {
        perror("An error ocurred while creating the lobby socket");
    }
    
    // Setting properties for lobby address
    sLobbyAddress.sin_family = AF_INET;
    sLobbyAddress.sin_port = htons(psArguments->siPort);
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
        psWorkerArguments->pcFilePath = psArguments->pcFilePath;
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
