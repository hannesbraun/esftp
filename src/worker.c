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

int getItemHeader(char* path, union ItemHeader* header);
int isFolderEmpty(char* path);
int sendItemViaTCP(int socketID, union ItemHeader* header, char* path);
int sendInner(int socketID, char* path);
ssize_t sendMultiple(int sockfd, const void* buf, size_t len, int flags, int tries);
ssize_t readMultiple(int fd, void* buf, size_t count, int tries);

/**
 * Sends the data to the connected client.
 */
void* worker(void* vConfig)
{
        // Cast to WorkerArguments for better access
        struct WorkerConfig* config = (struct WorkerConfig*) vConfig;

        // This worker is compliant with version 1 of the esftp protocol
        unsigned char headerByte = 1;

        // General purpose return value
        int tmp;

        unsigned long int i;

        union ItemHeader itemHeader;

        // Send the header byte
        tmp = sendMultiple(config->socketID, &headerByte, 1, 0, MAX_TRIES_EINTR);
        if (tmp == -1) {
                perror("An error ocurred while sending the header byte");
                goto error;
        }

        for (i = 0; i < config->itemsLen; i++) {
                tmp = getItemHeader(config->items[i], &itemHeader);
                if (tmp == -1) {
                        goto error;
                }

                if (i == (config->itemsLen - 1)) {
                        itemHeader.item.lastItem = 1;
                }

                // Send
                tmp = sendItemViaTCP(config->socketID, &itemHeader, config->items[i]);
                if (tmp == -1) {
                        goto error;
                }

                if (itemHeader.item.type == TYPE_FOLDER && itemHeader.item.emptyFolder == 0) {
                        tmp = sendInner(config->socketID, config->items[i]);
                        if (tmp == -1) {
                                goto error;
                        }
                }
        }

error:
        // Close socket
        tmp = close(config->socketID);
        if (tmp == -1) {
                perror("An error ocurred while closing the worker socket");
        }

        config->finished = 1;
        return NULL;
}

int getItemHeader(char* path, union ItemHeader* header)
{
        struct stat result;
        int tmp;
        int empty;

        (*header).byte = 0;

        tmp = stat(path, &result);
        if (tmp == -1) {
                perror("An error occurred while getting the filestats");
                return -1;
        }

        if (S_ISREG(result.st_mode)) {
                (*header).item.type = TYPE_FILE;
                (*header).item.emptyFolder = 0;
        } else if (S_ISDIR(result.st_mode)) {
                (*header).item.type = TYPE_FOLDER;

                // Check if folder is empty
                empty = isFolderEmpty(path);
                if (empty == 1) {
                        (*header).item.emptyFolder = 1;
                } else if (empty == 0) {
                        (*header).item.emptyFolder = 0;
                } else {
                        return -1;
                }
        }

        return 0;
}

int isFolderEmpty(char* path)
{
        int i = 0;
        struct dirent* entry;
        int tmp;

        DIR* dir = opendir(path);
        if (dir == NULL) {
                perror("An error ocurred while opening the directory for the empty-check");
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
                perror("An error ocurred while reading the directory contents in the empty-check");
                return -1;
        }

        tmp = closedir(dir);
        if (tmp == -1) {
                perror("An error ocurred while closing the directory after the empty-check");
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

/**
 * Sends an item via TCP
 */
int sendItemViaTCP(int socketID, union ItemHeader* header, char* path)
{
        int retVal = 0;
        int tmp;
        char pathCpy[4096] = {0};
        char* base;
        uint64_t size;
        int fd;
        unsigned char buf[BUFFERSIZE];
        // Read bytes at last read operation
        int readBytes;
        unsigned int nameLen;

        strncpy(pathCpy, path, strlen(path) + 1);
        base = basename(pathCpy);

        // Calculate filename length
        nameLen = (strlen(base)) / 128;
        if (nameLen > 31) {
                (*header).item.nameLen = 31;
                pathCpy[4095] = 0;
        } else {
                (*header).item.nameLen = nameLen;
        }

        // Header
        tmp = sendMultiple(socketID, &(*header).byte, 1, 0, MAX_TRIES_EINTR);
        if (tmp == -1) {
                perror("An error ocurred while sending the item header byte");
                retVal = -1;
                goto error;
        }

        // Item name
        tmp = sendMultiple(socketID, base, ((*header).item.nameLen + 1) * 128, 0, MAX_TRIES_EINTR);
        if (tmp == -1) {
                perror("An error ocurred while sending the item name");
                retVal = -1;
                goto error;
        }

        if ((*header).item.type == TYPE_FILE) {
                // File size

                tmp = calculateFileSize(path, &size);
                if (tmp == -1) {
                        retVal = -1;
                        goto error;
                }

                tmp = sendMultiple(socketID, &size, sizeof(size), 0, MAX_TRIES_EINTR);
                if (tmp == -1) {
                        perror("An error ocurred while sending the file size");
                        retVal = -1;
                        goto error;
                }

                // Open file
                fd = open(path, O_RDONLY);
                if (fd == -1) {
                        fprintf(stderr, "An error ocurred while opening the file %s: %s\n", path, strerror(errno));
                        retVal = -1;
                        goto error;
                }

                // File/data
                do {
                        // Reading
                        readBytes = readMultiple(fd, buf, BUFFERSIZE, MAX_TRIES_EINTR);
                        if (readBytes == -1) {
                                fprintf(stderr, "An error ocurred while reading the file %s: %s\n", path, strerror(errno));
                                retVal = -1;
                                goto errorAfterOpen;
                        }


                        if (readBytes > 0) {
                                // Sending File
                                tmp = sendMultiple(socketID, buf, readBytes, 0, MAX_TRIES_EINTR);
                                if (tmp == -1) {
                                        fprintf(stderr, "An error ocurred while sending the file %s: %s\n", path, strerror(errno));
                                        retVal = -1;
                                        goto errorAfterOpen;
                                }
                        }
                } while (readBytes > 0 && (serverShutdownState == noShutdown || serverShutdownState == friendlyShutdown));

errorAfterOpen:
                // Close file
                tmp = close(fd);
                if (tmp == -1) {
                        perror("An error ocurred while closing the file");
                        retVal = -1;
                }
        }

error:
        return retVal;

}

/**
 * Sends the contents of a directory
 */
int sendInner(int socketID, char* path)
{
        union ItemHeader header;
        DIR* dir;
        struct dirent* entry;
        int retVal = 0;
        char* pathCurr;
        char* tmpPtr;
        int tmp;

        dir = opendir(path);
        if (dir == NULL) {
                fprintf(stderr, "An error ocurred while opening the directory %s: %s\n", path, strerror(errno));
                retVal = -1;
                goto errorBeforeMalloc;
        }

        errno = 0;
        entry = readdir(dir);
        if (entry == NULL && errno != 0) {
                fprintf(stderr, "An error ocurred while reading the directory %s: %s\n", path, strerror(errno));
                retVal = -1;
                goto errorBeforeMalloc;
        }

        pathCurr = (char*) malloc((strlen(path) + 1 + strlen(entry->d_name) + 1) * sizeof(char));
        if (pathCurr == NULL) {
                fprintf(stderr, "An error ocurred while allocating memory for the path string.\n");
                retVal = -1;
                goto errorBeforeMalloc;
        }

        while (entry != NULL) {
                // Check for . and ..
                if (strncmp(entry->d_name, ".", 2) == 0 || strncmp(entry->d_name, "..", 3) == 0) {
                        errno = 0;
                        entry = readdir(dir);
                        // Error handling located after loop
                        continue;
                }

                // Allocate space for path
                tmpPtr = (char*) realloc(pathCurr, (strlen(path) + 1 + strlen(entry->d_name) + 1) * sizeof(char));
                if (pathCurr == NULL) {
                        fprintf(stderr, "An error ocurred while allocating memory for the path string.\n");
                        retVal = -1;
                        goto error;
                } else {
                        pathCurr = tmpPtr;
                }

                // Construct path string
                strncpy(pathCurr, path, strlen(path) + 1 + strlen(entry->d_name) + 1);
                if (pathCurr[strlen(pathCurr) - 1] != '/') {
                        strncat(pathCurr, "/", strlen(path) + 1 + strlen(entry->d_name) + 1);
                }
                strncat(pathCurr, entry->d_name, strlen(path) + 1 + strlen(entry->d_name) + 1);

                // Read next entry to determine if this is the last entry (excluding . and ..)
                errno = 0;
                do {
                        entry = readdir(dir);
                } while (entry != NULL && (strncmp(entry->d_name, ".", 2) == 0 || strncmp(entry->d_name, "..", 3) == 0));
                if (entry == NULL && errno != 0) {
                        fprintf(stderr, "An error ocurred while reading the directory %s: %s\n", path, strerror(errno));
                        retVal = -1;
                        goto error;
                }

                tmp = getItemHeader(pathCurr, &header);
                if (tmp == -1) {
                        retVal = -1;
                        goto error;
                }

                if (entry == NULL) {
                        header.item.lastItem = 1;
                } else {
                        header.item.lastItem = 0;
                }

                tmp = sendItemViaTCP(socketID, &header, pathCurr);
                if (tmp == -1) {
                        retVal = -1;
                        goto error;
                }

                if (header.item.type == TYPE_FOLDER && header.item.emptyFolder == 0) {
                        tmp = sendInner(socketID, pathCurr);
                        if (tmp == -1) {
                                retVal = -1;
                                goto error;
                        }
                }
        }
        if (errno != 0) {
                fprintf(stderr, "An error ocurred while reading the directory %s: %s\n", path, strerror(errno));
                retVal = -1;
                goto error;
        }

error:
        free(pathCurr);

errorBeforeMalloc:
        tmp = closedir(dir);
        if (tmp == -1) {
                fprintf(stderr, "An error ocurred while closing the directory %s: %s\n", path, strerror(errno));
                retVal = -1;
        }

        return retVal;
}

/**
 * Send a message on a socket with multiple tries in case of an interrupt
 */
ssize_t sendMultiple(int sockfd, const void* buf, size_t len, int flags, int tries)
{
        ssize_t tmp;

        do {
                tmp = send(sockfd, buf, len, flags);
                tries--;
        } while (tmp == -1 && tries > 0 && errno == EINTR);

        return tmp;
}

ssize_t readMultiple(int fd, void* buf, size_t count, int tries)
{
        ssize_t tmp;

        do {
                tmp = read(fd, buf, count);
                tries--;
        } while (tmp == -1 && tries > 0 && errno == EINTR);

        return tmp;
}