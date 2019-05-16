#include <arpa/inet.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "client.h"
#include "../util/recvExact.h"

void connectAndReceive(ClientArguments* psArguments)
{
    // General purpose return value
    int iReturnValue;
    
    int iSocketID;
    
    struct sockaddr_in sServerAddress;
    
    // Information to receive
    uint16_t ui16FileNameLength;
    uint64_t ui64FileSize;
    
    // File name
    char* pcFileName;
    
    // Data buffer
    char acDataBuffer[RECVBUFFERSIZE];
    unsigned int uiCurrentBufferSize;
    
    // File descriptor
    int iFileDescriptor;
    
    int iBytesReceived;
    unsigned long int uliBytesLeft;
    
    iSocketID = socket(AF_INET, SOCK_STREAM, 0);
    
    // Setting properties for lobby address
    sServerAddress.sin_family = AF_INET;
    sServerAddress.sin_port = htons(psArguments->siPort);
    sServerAddress.sin_addr = psArguments->sIPAddress;
    
    // Connecting
    iReturnValue = connect(iSocketID, (struct sockaddr*) &sServerAddress, sizeof(sServerAddress));
    if (iReturnValue == -1)
    {
        perror("An error ocurred while connecting to the client");
    }
    
    // Getting file name length
    iReturnValue = recvExact(iSocketID, &ui16FileNameLength, sizeof(ui16FileNameLength), 0);
    if (iReturnValue == -1)
    {
        perror("An error ocurred while receiving the length of the file name");
    }
    
    // Allocating memory for file name
    pcFileName = (char*) malloc(ui16FileNameLength * sizeof(char));
    if (pcFileName == NULL)
    {
        perror("An error ocurred while allocating memory for the file name");
    }
    
    // Getting file name length
    iReturnValue = recvExact(iSocketID, pcFileName, ui16FileNameLength * sizeof(char), 0);
    if (iReturnValue == -1)
    {
        perror("An error ocurred while receiving the file name");
    }
    
    // Getting file size
    iReturnValue = recvExact(iSocketID, &ui64FileSize, sizeof(ui64FileSize), 0);
    if (iReturnValue == -1)
    {
        perror("An error ocurred while receiving the file size");
    }
    uliBytesLeft = ui64FileSize;
    
    // Opening the file
    iFileDescriptor = open(pcFileName, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (iFileDescriptor == -1)
    {
        perror("An error ocurred while opening the new file");
    }
    
    // Receiving the file
    while (uliBytesLeft > 0)
    {
        if (uliBytesLeft > RECVBUFFERSIZE)
        {
            uiCurrentBufferSize = RECVBUFFERSIZE;
        }
        else
        {
            uiCurrentBufferSize = uliBytesLeft;
        }
        
        iBytesReceived = recv(iSocketID, acDataBuffer, uiCurrentBufferSize, 0);
        
        // Write to file
        iReturnValue = write(iFileDescriptor, acDataBuffer, iBytesReceived);
        if (iReturnValue == -1)
        {
            perror("An error ocurred while writing the received data to the file");
        }
        
        // Update bytes left
        uliBytesLeft = uliBytesLeft - iBytesReceived;
    }
    
    // Closing the socket
    iReturnValue = close(iSocketID);
    if (iReturnValue == -1)
    {
        perror("An error ocurred while closing the socket");
    }
    
    // Closing file
    iReturnValue = close(iFileDescriptor);
    if (iReturnValue == -1)
    {
        perror("An error ocurred while closing the file");
    }
    
    free(pcFileName);
}
