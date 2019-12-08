/**
 * @file worker.c
 * @brief File contains the worker code for the server to actually send the data to a client.
 * @author Hannes Braun
 * @date 18.06.2019
 */

#define _FILE_OFFSET_BITS 64

#include "worker.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "server.h"
#include "fileSize.h"

/**
 * Sends the data to the connected client.
 */
void* worker(void* vConfig)
{
        // Cast to WorkerArguments for better access
        struct WorkerConfig* config = (struct WorkerConfig*) vConfig;

        // Buffer for reading file
        char buf[BUFFERSIZE];

        // Read bytes at last read operation
        int readBytes;

        // Flag to indicate skipping the read operation (because of EINTR)
        unsigned char skipRead = 0;

        // File size and file name length to send
        int64_t fileSize = calculateFileSize(config->pcFilePath);
        uint16_t fileNameLength = strlen(config->pcFileName) + 1;

	unsigned char headerByte = 1;

        // General purpose return value
        int retVal;

        // Open the file
        int fd = open(psWorkerArguments->pcFilePath, O_RDONLY);
        if (fd == -1) {
                perror("An error ocurred while opening the file");
                goto errorDuringOpening;
        }

	// Send the header byte
	retVal = send(config->socketID, &headerByte, 1, 0);
	if (retVal == -1) {
	  perror("An error ocurred while sending the header byte");
	  goto errorDuringTransmission;
	}

        // Send length of file name string (including terminating null character)
        retVal = send(config->socketID, &fileNameLength, sizeof(fileNameLength), 0);
        if (iReturnValue == -1) {
                perror("An error ocurred while sending the length of the file name string");
                goto errorDuringTransmission;
        }

        // Send file name
        iReturnValue = send(config->socketID, config->fileName, strlen(config->fileName) + 1, 0);
        if (retVal == -1) {
                perror("An error ocurred while sending the file name");
                goto errorDuringTransmission;
        }

        // Send file size
        retVal = send(psWorkerArguments->iWorkerSocketID, &i64FileSize, sizeof(i64FileSize), 0);
        if (retVal == -1) {
                perror("An error ocurred while sending the fize size");
                goto errorDuringTransmission;
        }

        do {
                if (skipRead == 0) {
                        // Reading
                        readBytes = read(f, buf, BUFFERSIZE);
                        if (readBytes == -1) {
                                if (errno == EINTR) {
                                        // Interrupted, try again
                                        continue;
                                }
                                perror("An error ocurred while reading the file");
                                goto errorDuringTransmission;
                        }
                }

                if (readBytes > 0) {
                        // Sending File
                        retVal = send(config->socketID, buf, readBytes, 0);
                        if (retVal == -1) {
                                if (errno == EINTR) {
                                        // Interrupted, try again but skip read operation
                                        skipRead = 1;
                                        continue;
                                }
                                perror("An error ocurred while sending the file");
                                goto errorDuringTransmission;
                        }
                        skipRead = 0;
                }
        } while (readBytes > 0 && (serverShutdownState == noShutdown || serverShutdownState == friendlyShutdown));

errorDuringTransmission:

        // Close file
        retVal = close(fd);
        if (retVal == -1)
                perror("An error ocurred while closing the file");

errorDuringOpening:

        // Close socket
        retVal = close(config->socketID);
        if (retVal == -1)
                perror("An error ocurred while closing the worker socket");

        config->finished = 1;
        return NULL;
}
