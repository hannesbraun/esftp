/**
 * @file worker.c
 * @brief File contains the worker code for the server to actually send the data to a client.
 * @author Hannes Braun
 * @date 18.06.2019
 */

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "server.h"
#include "../util/fileSize.h"

/**
 * @fn void* worker(void* pvArguments)
 * @brief Sends the file (and the metadata) to the connected client.
 * @param pvArguments the arguments to run this worker with (provided as a WorkerArguments struct)
 * @return void* This function is just returning NULL to satisfy pthread.
 * @author Hannes Braun
 * @date 07.06.2019
 */
void* worker(void* pvArguments)
{
    // Cast to WorkerArguments for better access
    WorkerArguments* psWorkerArguments = (WorkerArguments*) pvArguments;

    // Buffer for reading file
    char acBuffer[BUFFERSIZE];

    // Read bytes at last read operation
    int iReadBytes;

    // File size and file name length to send
    int64_t i64FileSize = calculateFileSize(psWorkerArguments->pcFilePath);
    uint16_t ui16FileNameLength = strlen(psWorkerArguments->pcFileName) + 1;

    // General purpose return value
    int iReturnValue;

    // Open the file
    int iFileDescriptor = open(psWorkerArguments->pcFilePath, O_RDONLY);
    if (iFileDescriptor == -1)
    {
        perror("An error ocurred while opening the file");
    }

    // Send length of file name string (including terminating null character)
    iReturnValue = send(psWorkerArguments->iWorkerSocketID, &ui16FileNameLength, sizeof(ui16FileNameLength), 0);
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
    iReturnValue = send(psWorkerArguments->iWorkerSocketID, &i64FileSize, sizeof(i64FileSize), 0);
    if (iReturnValue == -1)
    {
        perror("An error ocurred while sending the fize size");
    }

    do
    {
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
    if (iReturnValue == -1)
    {
        perror("An error ocurred while closing the file");
    }

    // Free memory for arguments
    free(pvArguments);

    return NULL;
}
