#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "server.h"

void* worker(void* pvArguments)
{
    // Cast to WorkerArguments for better access
    WorkerArguments* psWorkerArguments = (WorkerArguments*) pvArguments;
    
    char acBuffer[BUFFERSIZE];
    int iReadBytes;
    
    // General purpose return value
    int iReturnValue;
    
    int iFileDescriptor = open(psWorkerArguments->pcFilePath, O_RDONLY);
    if (iFileDescriptor == -1)
    {
        perror("An error ocurred while opening the file");
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
