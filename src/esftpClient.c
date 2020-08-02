/**
 * @file esftp_client.c
 * @brief File contains the actual function to receive the data from the server.
 * @author Hannes Braun
 * @date 16.06.2019
 */

#define _FILE_OFFSET_BITS 64

#include <arpa/inet.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "commons.h"
#include "client.h"
#include "recvExact.h"

#define STATUS_STRING_SIZE 80

enum ByteUnit {
        Byte,
        Kibibyte,
        Mebibyte,
        Gibibyte,
        Tebibyte,
        Pebibyte,
        Exbibyte
};

int recvLevel(int socketID);
int recvFile(int socketID, uint64_t size, char* name);
void printStatus(uint64_t* byteProgress, struct timeval* timeProgress, uint64_t size, unsigned char erase);
enum ByteUnit getUnit(uint64_t size);
uint64_t convertBytes(uint64_t bytes, enum ByteUnit unit, unsigned int* dec);
void shiftDec(unsigned int* dec, uint64_t in);
void getUnitStr(enum ByteUnit unit, char* str);

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
int connectAndReceive(struct Config* config)
{
        // General purpose return value
        int tmp;
        int retVal = 0;

        int socketID;

        struct sockaddr_in serverAddr;

        // Information to receive
        unsigned char headerByte;

        socketID = socket(AF_INET, SOCK_STREAM, 0);

        // Setting properties for lobby address
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(config->port);
        serverAddr.sin_addr = config->addr;

        // Connecting
        printf("Connecting to %s... ", inet_ntoa(serverAddr.sin_addr));
        tmp = connect(socketID, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
        if (tmp == -1) {
                printf("failed\n");
                perror("An error ocurred while connecting to the client");
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
                // Safety first, ensure the null byte at the end of the name and at max name length position
                name[4095] = 0;
                if (NAME_MAX < 4096) {
                        name[NAME_MAX] = 0;
                }

                if (header.item.type == TYPE_DIRECTORY) {
                        // Directory

                        // Create
                        tmp = mkdir(name, 0750);
                        if (tmp == -1) {
                                perror("An error ocurred while creating the directory");
                                retVal = -1;
                                goto error;
                        }

                        if (header.item.emptyDirectory == 0) {
                                // Receive directory content
                                tmp = chdir(name);
                                if (tmp == -1) {
                                        perror("An error ocurred while changing the directory");
                                        retVal = -1;
                                        goto error;
                                }

                                tmp = recvLevel(socketID);
                                if (tmp == -1) {
                                        retVal = -1;
                                        goto error;
                                }

                                tmp = chdir("..");
                                if (tmp == -1) {
                                        perror("An error ocurred while changing the directory");
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

int recvFile(int socketID, uint64_t size, char* name)
{
        int tmp;
        int retVal = 0;

        uint64_t bytesLeft = size;
        int bytesRecv;

        int fd;
        unsigned char buf[RECVBUFFERSIZE];
        unsigned int bufSize;

        uint64_t byteProgress[21] = {0};
        struct timeval timeProgress[21];
        struct timeval timeCurrent;
        uint64_t timeDiff;
        int i;

        // Opening the file
        fd = open(name, O_WRONLY | O_CREAT | O_EXCL, 0640);
        if (fd == -1) {
                perror("An error ocurred while opening the new file");
                return -1;
        }

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

                        printStatus(byteProgress, timeProgress, size, 0);
                }
        }

        printStatus(byteProgress, timeProgress, size, 1);

error:
        // Closing file
        tmp = close(fd);
        if (tmp == -1) {
                perror("An error ocurred while closing the file");
                retVal = -1;
        }

        return retVal;
}

void printStatus(uint64_t* byteProgress, struct timeval* timeProgress, uint64_t size, unsigned char erase)
{
        unsigned int i;
        enum ByteUnit unitSize;
        enum ByteUnit unitSpeed;
        uint64_t transferred;
        uint64_t timeDiff;
        uint64_t speed;
        unsigned int dec;
        unsigned int min;
        unsigned int sec;
        uint64_t bytesLeft = size - byteProgress[20];
        char out[1024];
        char unitSizeStr[4];
        char unitSpeedStr[4];

        // Get file size unit
        unitSize = getUnit(size);
        getUnitStr(unitSize, unitSizeStr);

        // Calculate bytes transfered and time difference
        transferred = byteProgress[20] - byteProgress[0];
        timeDiff = (timeProgress[20].tv_sec * 1000000 + timeProgress[20].tv_usec) - (timeProgress[0].tv_sec * 1000000 + timeProgress[0].tv_usec);

        // Calculate speed in bytes per second and eta
        speed = (transferred * 1000000) / timeDiff;
        if (speed > 0) {
                min = (bytesLeft / speed) / 60;
                sec = (bytesLeft / speed) % 60;
        } else {
                // Avoid division by zero
                min = UINT_MAX;
                sec = UINT_MAX;
        }

        // Get speed unit
        unitSpeed = getUnit(transferred);
        transferred = convertBytes(transferred, unitSpeed, &dec);
        getUnitStr(unitSpeed, unitSpeedStr);

        // Write new status
        snprintf(out, 1024, "%" PRIu64 " %s / %" PRIu64 " %s | %" PRIu64 ",%llu %s/s | ETA: %dm %ds   ",
                 convertBytes(byteProgress[20], unitSize, &dec),
                 unitSizeStr,
                 convertBytes(size, unitSize, &dec),
                 unitSizeStr,
                 (transferred * 1000000) / timeDiff,
                 (dec * 1000000) / timeDiff,
                 unitSpeedStr,
                 min,
                 sec);

        // Move cursor backwards
        printf("\x1b[1024D");
        if (erase == 0) {
                // Update status
                printf("%s", out);
        } else if (erase == 1) {
                // Erase status
                for (i = 0; i < strlen(out); i++) {
                        putchar(' ');
                }
                printf("\x1b[1024D");
        }

        fflush(stdout);
}

enum ByteUnit getUnit(uint64_t size)
{
        if (size < 1000) {
                return Byte;
        } else if (size < 1000000) {
                return Kibibyte;
        } else if (size < 1000000000) {
                return Mebibyte;
        } else if (size < 1000000000000) {
                return Gibibyte;
        } else if (size < 1000000000000000) {
                return Tebibyte;
        } else if (size < 1000000000000000000) {
                return Pebibyte;
        } else {
                return Exbibyte;
        }
}

uint64_t convertBytes(uint64_t bytes, enum ByteUnit unit, unsigned int* dec)
{
        uint64_t retVal = 0;
        uint64_t tmpDec;

        switch (unit) {
                case Byte:
                        retVal = bytes;
                        (*dec) = 0;
                        break;
                case Kibibyte:
                        retVal = bytes >> 10;
                        tmpDec = bytes - (retVal << 10);
                        shiftDec(dec, tmpDec);
                        break;
                case Mebibyte:
                        retVal = bytes >> 20;
                        tmpDec = bytes - (retVal << 20);
                        shiftDec(dec, tmpDec);
                        break;
                case Gibibyte:
                        retVal = bytes >> 30;
                        tmpDec = bytes - (retVal << 30);
                        shiftDec(dec, tmpDec);
                        break;
                case Tebibyte:
                        retVal = bytes >> 40;
                        tmpDec = bytes - (retVal << 40);
                        shiftDec(dec, tmpDec);
                        break;
                case Pebibyte:
                        retVal = bytes >> 50;
                        tmpDec = bytes - (retVal << 50);
                        shiftDec(dec, tmpDec);
                        break;
                case Exbibyte:
                        retVal = bytes >> 60;
                        tmpDec = bytes - (retVal << 60);
                        shiftDec(dec, tmpDec);
                        break;
        }

        return retVal;
}

void shiftDec(unsigned int* dec, uint64_t in)
{
        while (in >= 100) {
                in = in / 10;
        }

        (*dec) = in;
}

void getUnitStr(enum ByteUnit unit, char* str)
{
        switch (unit) {
                case Byte:
                        strncpy(str, "B", 2);
                        break;
                case Kibibyte:
                        strncpy(str, "KiB", 4);
                        break;
                case Mebibyte:
                        strncpy(str, "MiB", 4);
                        break;
                case Gibibyte:
                        strncpy(str, "GiB", 4);
                        break;
                case Tebibyte:
                        strncpy(str, "TiB", 4);
                        break;
                case Pebibyte:
                        strncpy(str, "PiB", 4);
                        break;
                case Exbibyte:
                        strncpy(str, "EiB", 4);
                        break;
        }
}
