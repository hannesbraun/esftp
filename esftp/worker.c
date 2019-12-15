/**
 * @file worker.c
 * @brief File contains the worker code for the server to actually send the data to a client.
 * @author Hannes Braun
 * @date 18.06.2019
 */

#define _FILE_OFFSET_BITS 64

#include "worker.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "commons.h"
#include "server.h"
#include "serverConfig.h"
#include "fileSize.h"

union ItemHeader getItemHeader(char* path);
int isFolderEmpty(char* path);
int sendItemViaTCP(int socketID, union ItemHeader* header, char* path);
int sendInner(int socketID, char* path);

/**
 * Sends the data to the connected client.
 */
void* worker(void* vConfig)
{
        // Cast to WorkerArguments for better access
        struct WorkerConfig* config = (struct WorkerConfig*) vConfig;

        unsigned char headerByte = 1;

        // General purpose return value
        int retVal;

        unsigned long int i;

        union ItemHeader itemHeader;

        // Open the file
        //int fd = open(psWorkerArguments->pcFilePath, O_RDONLY);
        //if (fd == -1) {
        //        perror("An error ocurred while opening the file");
        //        goto errorDuringOpening;
        //}

        // Send the header byte
        retVal = send(config->socketID, &headerByte, 1, 0);
        if (retVal == -1) {
                perror("An error ocurred while sending the header byte");
                goto errorDuringTransmission;
        }

        for (i = 0; i < config->itemsLen; i++) {
                itemHeader = getItemHeader(config->items[i]);
                if (i == (config->itemsLen - 1)) {
                        itemHeader.item.lastItem = 1;
                }

                // Send
                retVal = sendItemViaTCP(config->socketID, &itemHeader, config->items[i]);
                if (retVal == -1) {

                }

                if (itemHeader.item.type == TYPE_FOLDER && itemHeader.item.emptyFolder == 0) {
                        sendInner(config->socketID, config->items[i]);
                }
        }

        // Send length of file name string (including terminating null character)
        //retVal = send(config->socketID, &fileNameLength, sizeof(fileNameLength), 0);
        //if (iReturnValue == -1) {
        //        perror("An error ocurred while sending the length of the file name string");
        //        goto errorDuringTransmission;
        //}

        // Send file name
        //iReturnValue = send(config->socketID, config->fileName, strlen(config->fileName) + 1, 0);
        //if (retVal == -1) {
        //        perror("An error ocurred while sending the file name");
        //        goto errorDuringTransmission;
        //}

        // Send file size
        //retVal = send(psWorkerArguments->iWorkerSocketID, &i64FileSize, sizeof(i64FileSize), 0);
        //if (retVal == -1) {
        //        perror("An error ocurred while sending the fize size");
        //        goto errorDuringTransmission;
        //}


errorDuringTransmission:

        // Close file
        //retVal = close(fd);
        //if (retVal == -1)
        //{
        //        perror("An error ocurred while closing the file");
        //}

errorDuringOpening:

        // Close socket
        retVal = close(config->socketID);
        if (retVal == -1) {
                perror("An error ocurred while closing the worker socket");
        }

        config->finished = 1;
        return NULL;
}

union ItemHeader getItemHeader(char* path)
{
        struct stat result;
        union ItemHeader header;
        int retVal;
        int empty;

        header.byte = 0;

        retVal = stat(path, &result);
        if (retVal == -1) {
                perror("An error occurred while getting the filestats");
        }

        if (S_ISREG(result.st_mode)) {
                header.item.type = TYPE_FILE;
                header.item.emptyFolder = 0;
        } else if (S_ISDIR(result.st_mode)) {
                header.item.type = TYPE_FOLDER;

                // Check if folder is empty
                empty = isFolderEmpty(path);
                if (empty == 1) {
                        header.item.emptyFolder = 1;
                } else if (empty == 0) {
                        header.item.emptyFolder = 0;
                } else if (empty == -1) {
                        perror("An error ocurred while checking if the folder is empty");
                } else {
                        // Unexpected return value, should never happen
                        fprintf(stderr, "The function isFolderEmpty returned an unexpected value.");
                }
        }

        return header;
}

int isFolderEmpty(char* path)
{
        int i = 0;
        struct dirent* entry;
        int retVal;

        DIR* dir = opendir(path);
        if (dir == NULL) {
                return -1;
        }

        errno = 0;

        // Check if there are more items than . and ..
        while ((entry = readdir(dir)) != NULL) {
                i++;
                if (i > 2) {
                        // Folder is not empty
                        break;
                }
        }

        if (errno != 0) {
                // readdir failed
                return -1;
        }

        retVal = closedir(dir);
        if (retVal == -1) {
                return -1;
        }

        if (i > 2) {
                // Not empty
                return 0;
        } else {
                // Empty
                return 1;
        }
}

int sendItemViaTCP(int socketID, union ItemHeader* header, char* path)
{
        int retVal;
        char* pathCpy;
        char* base;
        int64_t size;
        int fd;
        unsigned char buf[BUFFERSIZE];
        // Flag to indicate skipping the read operation (because of EINTR)
        unsigned char skipRead = 0;
        // Read bytes at last read operation
        int readBytes;

        pathCpy = (char*) malloc((strlen(path) + 1) * sizeof(char));
        if (pathCpy == NULL) {

        }

        strncpy(pathCpy, path, strlen(path) + 1);
        base = basename(pathCpy);

        // Header
        retVal = send(socketID, &(*header).byte, 1, 0);
        if (retVal == -1) {

        }

        // Filename
        retVal = send(socketID, base, strlen(base) + 1, 0);
        if (retVal == -1) {

        }

        if ((*header).item.type == TYPE_FILE) {
                // Filename

                size = calculateFileSize(path);
                if (size == -1) {

                }

                retVal = send(socketID, &size, sizeof(size), 0);
                if (retVal == -1) {

                }

                // Open file
                fd = open(path, O_RDONLY);
                if (fd == -1) {

                }

                // File/data
                do {
                        if (skipRead == 0) {
                                // Reading
                                readBytes = read(fd, buf, BUFFERSIZE);
                                if (readBytes == -1) {
                                        if (errno == EINTR) {
                                                // Interrupted, try again
                                                continue;
                                        }
                                        perror("An error ocurred while reading the file");
                                        // goto errorDuringTransmission;
                                }
                        }

                        if (readBytes > 0) {
                                // Sending File
                                retVal = send(socketID, buf, readBytes, 0);
                                if (retVal == -1) {
                                        if (errno == EINTR) {
                                                // Interrupted, try again but skip read operation
                                                skipRead = 1;
                                                continue;
                                        }
                                        perror("An error ocurred while sending the file");
                                        // goto errorDuringTransmission;
                                }
                                skipRead = 0;
                        }
                } while (readBytes > 0 && (serverShutdownState == noShutdown || serverShutdownState == friendlyShutdown));

                // Close file
                retVal = close(fd);
                if (retVal == -1) {
                        perror("An error ocurred while closing the file");
                }
        }

        free(pathCpy);

        return retVal;

}

int sendInner(int socketID, char* path)
{
        union ItemHeader header;
        DIR* dir;
        struct dirent* entry;
        int retVal;
        char* pathCurr;

        dir = opendir(path);
        if (dir == NULL) {

        }

        errno = 0;

        entry = readdir(dir);

        pathCurr = (char*) malloc((strlen(path) + 1 + strlen(entry->d_name) + 1) * sizeof(char));
        if (pathCurr == NULL) {

        }


        while (entry != NULL) {
                // Check for .. and ..
                if (strncmp(entry->d_name, ".", 2) == 0 || strncmp(entry->d_name, "..", 3) == 0) {
                        entry = readdir(dir);
                        continue;
                }

                // Allocate space for path
                pathCurr = (char*) realloc(pathCurr, (strlen(path) + 1 + strlen(entry->d_name) + 1) * sizeof(char));
                if (pathCurr == NULL) {

                }

                // Construct path string
                strncpy(pathCurr, path, strlen(path) + 1 + strlen(entry->d_name) + 1);
                if (pathCurr[strlen(pathCurr) - 1] != '/') {
                        strncat(pathCurr, "/", strlen(path) + 1 + strlen(entry->d_name) + 1);
                }
                strncat(pathCurr, entry->d_name, strlen(path) + 1 + strlen(entry->d_name) + 1);

                // Read next entry to determine if this is the last entry (excluding . and ..)
                do {
                        entry = readdir(dir);
                } while (entry != NULL && (strncmp(entry->d_name, ".", 2) == 0 || strncmp(entry->d_name, "..", 3) == 0));

                header = getItemHeader(pathCurr);

                if (entry == NULL) {
                        header.item.lastItem = 1;
                } else {
                        header.item.lastItem = 0;
                }

                retVal = sendItemViaTCP(socketID, &header, pathCurr);
                if (retVal == -1) {

                }

                if (header.item.type == TYPE_FOLDER && header.item.emptyFolder == 0) {
                        sendInner(socketID, pathCurr);
                }
        }
        if (errno != 0) {

        }
        free(pathCurr);

        retVal = closedir(dir);
        if (retVal == -1) {

        }
}
