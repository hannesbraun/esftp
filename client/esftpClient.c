/**
 * @file esftp_client.c
 * @brief File contains the actual function to receive the data from the server.
 * @author Hannes Braun
 * @date 16.06.2019
 */

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

/**
 * @fn void parseArguments(int argc, char* argv[], ClientArguments* psArguments)
 * @brief This function parses the command line arguments.
 * @param argc the amount of command line arguments
 * @param argv the command line arguments
 * @param psArguments the struct to write the parsed data to
 * @return void
 * @author Hannes Braun
 * @date 16.06.2019
 */
void connectAndReceive(ClientConfiguration* psConfiguration)
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

    int64_t i64BytesReceived;
    uint64_t ui64BytesLeft;

    iSocketID = socket(AF_INET, SOCK_STREAM, 0);

    // Setting properties for lobby address
    sServerAddress.sin_family = AF_INET;
    sServerAddress.sin_port = htons(psConfiguration->siPort);
    sServerAddress.sin_addr = psConfiguration->sIPAddress;

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
    ui64BytesLeft = ui64FileSize;

    // Opening the file
    if (psConfiguration->pcOutputFileName == NULL)
    {
        iFileDescriptor = open(pcFileName, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    }
    else
    {
        // Override sent file name
        iFileDescriptor = open(psConfiguration->pcOutputFileName, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    }
    if (iFileDescriptor == -1)
    {
        perror("An error ocurred while opening the new file");
    }

    // Receiving the file
    while (ui64BytesLeft > 0)
    {
        if (ui64BytesLeft > RECVBUFFERSIZE)
        {
            uiCurrentBufferSize = RECVBUFFERSIZE;
        }
        else
        {
            uiCurrentBufferSize = ui64BytesLeft;
        }

        i64BytesReceived = recv(iSocketID, acDataBuffer, uiCurrentBufferSize, 0);
        
        // Write to file
        iReturnValue = write(iFileDescriptor, acDataBuffer, i64BytesReceived);
        if (iReturnValue == -1)
        {
            perror("An error ocurred while writing the received data to the file");
        }

        // Update bytes left
        ui64BytesLeft = ui64BytesLeft - i64BytesReceived;
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
