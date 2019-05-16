#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "server.h"
#include "../util/fileSize.h"

void* worker(void* pvArguments)
{
    // Cast to WorkerArguments for better access
    WorkerArguments* psWorkerArguments = (WorkerArguments*) pvArguments;
    
    char acBuffer[BUFFERSIZE];
    int iReadBytes;
    unsigned long int uliFileSize = calculateFileSize(psWorkerArguments->pcFilePath);
    unsigned short int usiFileNameLength = strlen(psWorkerArguments->pcFileName) + 1;
    
    // General purpose return value
    int iReturnValue;
    
    int iFileDescriptor = open(psWorkerArguments->pcFilePath, O_RDONLY);
    if (iFileDescriptor == -1)
    {
        perror("An error ocurred while opening the file");
    }
    
    // Send length of file name string (including terminating null character)
    iReturnValue = send(psWorkerArguments->iWorkerSocketID, &usiFileNameLength, sizeof(usiFileNameLength), 0);
    if (iReturnValue == -1)
    {
        perror("An error ocurred while sending the length of the file name string");
    }
    
    // Send file name
    iReturnValue = send(psWorkerArguments->iWorkerSocketID, psWorkerArguments->pcFileName, strlen(psWorkerArguments->pcFileName) + 1, 0);
    if (iReturnValue == -1)
    {
        perror("An error ocurred while sending the file name");
    }
    
    // Send file size
    iReturnValue = send(psWorkerArguments->iWorkerSocketID, &uliFileSize, sizeof(uliFileSize), 0);
    if (iReturnValue == -1)
    {
        perror("An error ocurred while sending the fize size");
    }
    
    do {
        // Reading
        iReadBytes = read(iFileDescriptor, acBuffer, BUFFERSIZE);
        if (iReadBytes == -1)
        {
            perror("An error ocurred while reading the file");
        }
        
        if (iReadBytes > 0)
        {
            // Sending File
            iReturnValue = send(psWorkerArguments->iWorkerSocketID, acBuffer, iReadBytes, 0);
            if (iReturnValue == -1)
            {
                perror("An error ocurred while sending the file");
            }
        }
    } while (iReadBytes > 0);
    
    // Close socket
    iReturnValue = close(psWorkerArguments->iWorkerSocketID);
    if (iReturnValue == -1)
    {
        perror("An error ocurred while closing the worker socket");
    }
    
    // Close file
    iReturnValue = close(iFileDescriptor);
    if (iReturnValue == -1) {
        perror("An error ocurred while closing the file");
    }
    
    // Free memory for arguments
    free(pvArguments);
    
    return NULL;
}
