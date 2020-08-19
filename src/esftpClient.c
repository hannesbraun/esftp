/**
 * File contains the actual esftp client code to receive the data from the server.
 */

#define _FILE_OFFSET_BITS 64

#include <arpa/inet.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "commons.h"
#include "client.h"
#include "recvExact.h"
#include "recvFileStatus.h"

int recvLevel(int socketID);
int recvFile(int socketID, uint64_t size, char* name);

/**
 * Connects to a server and receives the data.
 */
int connectAndReceive(struct ClientConfig* config)
{
        // General purpose return variable
        int tmp;

        // Function return variable
        int retVal = 0;

        // Socket file descriptor
        int socketID;

        // Server address
        struct sockaddr_in serverAddr;

        // Header byte for whole transmission
        unsigned char headerByte;

        // Create socket
        socketID = socket(AF_INET, SOCK_STREAM, 0);

        // Setting properties for server address
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(config->port);
        serverAddr.sin_addr = config->addr;

        // Connecting
        printf("Connecting to %s... ", inet_ntoa(serverAddr.sin_addr));
        tmp = connect(socketID, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
        if (tmp == -1) {
                printf("failed\n");
                perror("An error ocurred while connecting to the server");
                retVal = -1;
                goto error;
        } else {
                printf("done\n");
        }

        // Getting header byte
        tmp = recvExact(socketID, &headerByte, 1u, 0);
        if (tmp == -1) {
                perror("An error ocurred while receiving the header byte");
                retVal = -1;
                goto error;
        }

        if (headerByte != 1) {
                fprintf(stderr, "The server is not compliant with the protocol version implemented in this client.\n");
                retVal = -1;
                goto error;
        }

        tmp = recvLevel(socketID);
        if (tmp == -1) {
                retVal = -1;
                goto error;
        }

error:
        // Closing the socket
        tmp = close(socketID);
        if (tmp == -1) {
                perror("An error ocurred while closing the socket");
        }

        return retVal;
}

/**
 * Receives one level.
 * This can be either the content of the root directory or the content of any subdirectory.
 */
int recvLevel(int socketID)
{
        int tmp;
        int retVal = 0;

        // Item header
        union ItemHeader header;

        // File name
        char name[4096] = {0};

        // File size
        uint64_t size;

        do {
                // Getting item header
                tmp = recvExact(socketID, &(header.byte), 1, 0);
                if (tmp == -1) {
                        perror("An error ocurred while receiving an item header");
                        retVal = -1;
                        goto error;
                }

                // Getting name
                tmp = recvExact(socketID, name, (header.item.nameLen + 1) * 128, 0);
                if (tmp == -1) {
                        perror("An error ocurred while receiving the file name");
                        retVal = -1;
                        goto error;
                }
                // Safety first: ensure null byte at the end of the name and at max name length position
                name[4095] = 0;
                if (NAME_MAX < 4096) {
                        name[NAME_MAX] = 0;
                }

                if (header.item.type == TYPE_DIRECTORY) {
                        // Directory

                        // Create
                        tmp = mkdir(name, 0750);
                        if (tmp == -1) {
                                perror("An error ocurred while creating a directory");
                                retVal = -1;
                                goto error;
                        }

                        if (header.item.emptyDirectory == 0) {
                                // Change working directory to new directory
                                tmp = chdir(name);
                                if (tmp == -1) {
                                        perror("An error ocurred while changing into a new directory");
                                        retVal = -1;
                                        goto error;
                                }

                                // Receive directory content
                                tmp = recvLevel(socketID);
                                if (tmp == -1) {
                                        retVal = -1;
                                        goto error;
                                }

                                // Go back to parent directory
                                tmp = chdir("..");
                                if (tmp == -1) {
                                        perror("An error ocurred while changing back to the parent directory");
                                        retVal = -1;
                                        goto error;
                                }
                        }
                } else {
                        // File

                        // Getting file size
                        tmp = recvExact(socketID, &size, sizeof(size), 0);
                        if (tmp == -1) {
                                perror("An error ocurred while receiving a file size");
                                retVal = -1;
                                goto error;
                        }

                        // Receive file data
                        tmp = recvFile(socketID, size, name);
                        if (tmp == -1) {
                                retVal = -1;
                                goto error;
                        }
                }

        } while (header.item.lastItem == 0);

error:
        return retVal;
}

/**
 * Receives a single file
 */
int recvFile(int socketID, uint64_t size, char* name)
{
        int tmp;
        int retVal = 0;
        int i;

        // Number of bytes left to receive
        uint64_t bytesLeft = size;

        // Number of bytes received per operation of recv
        int bytesRecv;

        // File descriptor for file to receive
        int fd;

        // Receive buffer
        unsigned char buf[RECVBUFFERSIZE];

        // Buffer size for the recv function (max amount of bytes to receive)
        unsigned int bufSize;

        // States of the time and bytes received at the last 20 times the status
        // was printed
        uint64_t byteProgress[21] = {0};
        struct timeval timeProgress[21];

        // Current timestamp
        struct timeval timeCurrent;

        // Time between last status print and now in usecs
        uint64_t timeDiff;

        // Opening the file
        fd = open(name, O_WRONLY | O_CREAT | O_EXCL, 0640);
        if (fd == -1) {
                perror("An error ocurred while opening the new file");
                return -1;
        }

        // Get first timestamp for speed measurement/status
        tmp = gettimeofday(&(timeProgress[20]), NULL);
        if (tmp == -1) {
                perror("An error ocurred while getting the time of the day");
                retVal = -1;
                goto error;
        }

        // Receiving the file
        printf("Receiving file %s...\n", name);
        while (bytesLeft > 0) {
                if (bytesLeft > RECVBUFFERSIZE) {
                        bufSize = RECVBUFFERSIZE;
                } else {
                        bufSize = bytesLeft;
                }

                // Receive as much data as possible
                bytesRecv = recv(socketID, buf, bufSize, 0);
                if (bytesRecv == -1) {
                        perror("An error ocurred while receiving the file");
                        retVal = -1;
                        goto error;
                }

                // Write to file
                tmp = write(fd, buf, bytesRecv);
                if (tmp == -1) {
                        perror("An error ocurred while writing the received data to the file");
                        retVal = -1;
                        goto error;
                }

                // Update bytes left
                bytesLeft = bytesLeft - bytesRecv;

                // Update current time
                tmp = gettimeofday(&timeCurrent, NULL);
                if (tmp == -1) {
                        perror("An error ocurred while getting the time of the day");
                        retVal = -1;
                        goto error;
                }
                timeDiff = (timeCurrent.tv_sec * 1000000 + timeCurrent.tv_usec) - (timeProgress[20].tv_sec * 1000000 + timeProgress[20].tv_usec);

                // Update status every 50 ms
                if (timeDiff > 50000) {
                        for (i = 0; i < 20; i++) {
                                byteProgress[i] = byteProgress[i + 1];
                                timeProgress[i] = timeProgress[i + 1];
                        }

                        byteProgress[20] = size - bytesLeft;
                        timeProgress[20] = timeCurrent;

                        // Print status
                        printStatus(byteProgress, timeProgress, size, 0);
                }
        }

        // Erase printed status
        printStatus(byteProgress, timeProgress, size, 1);

error:
        // Close file
        tmp = close(fd);
        if (tmp == -1) {
                perror("An error ocurred while closing the file");
                retVal = -1;
        }

        return retVal;
}
