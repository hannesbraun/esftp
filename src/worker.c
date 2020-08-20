/******************************************************************************
 * Copyright 2019-2020 Hannes Braun
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 ******************************************************************************/

/**
 * File contains the worker code for the server to actually send the data to a client.
 */

#define _FILE_OFFSET_BITS 64

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "commons.h"
#include "fileSize.h"
#include "server.h"
#include "serverConfig.h"
#include "worker.h"

int getItemHeader(char* path, union ItemHeader* header);
int isDirectoryEmpty(char* path);
int sendItemViaTCP(int* socketID, union ItemHeader* header, char* path);
int sendInner(int* socketID, char* path);
ssize_t sendMultiple(int* sockfd, const void* buf, size_t len, int flags, int tries);
ssize_t readMultiple(int fd, void* buf, size_t count, int tries);

/**
 * Sends the data to the connected client.
 */
void* worker(void* vConfig)
{
        // Cast to WorkerConfig* for better access
        struct WorkerConfig* config = (struct WorkerConfig*) vConfig;

        // This worker is compliant with version 1 of the esftp protocol
        unsigned char headerByte = 1;

        // General purpose variable
        int tmp;

        unsigned long int i;

        union ItemHeader itemHeader;

        // Send the header byte
        tmp = sendMultiple(&(config->socketID), &headerByte, 1, 0, MAX_TRIES_EINTR);
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

                // Send item
                tmp = sendItemViaTCP(&(config->socketID), &itemHeader, config->items[i]);
                if (tmp == -1) {
                        goto error;
                }

                if (itemHeader.item.type == TYPE_DIRECTORY && itemHeader.item.emptyDirectory == 0) {
                        // Send inner content
                        tmp = sendInner(&(config->socketID), config->items[i]);
                        if (tmp == -1) {
                                goto error;
                        }
                }
        }

error:
        // Close socket if not already closed by error handling in lobby
        if (config->socketID != -1) {
                tmp = close(config->socketID);
                if (tmp == -1) {
                        perror("An error ocurred while closing the worker socket");
                }
        }

        if (config->selfFree == 0) {
                config->finished = 1;
        } else {
                free(vConfig);
        }

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
                // File
                (*header).item.type = TYPE_FILE;
                (*header).item.emptyDirectory = 0;
        } else if (S_ISDIR(result.st_mode)) {
                // Directory
                (*header).item.type = TYPE_DIRECTORY;

                // Check if directory is empty
                empty = isDirectoryEmpty(path);
                if (empty == 1) {
                        (*header).item.emptyDirectory = 1;
                } else if (empty == 0) {
                        (*header).item.emptyDirectory = 0;
                } else {
                        return -1;
                }
        }

        return 0;
}

/**
 * Checks if the given directory is empty.
 * Returns 1 if the directory is empty and 0 if the directory is not empty.
 * If an error occurs, -1 will be returned.
 */
int isDirectoryEmpty(char* path)
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
                        // Directory is not empty
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
int sendItemViaTCP(int* socketID, union ItemHeader* header, char* path)
{
        // Function return value
        int retVal = 0;

        // General purpose variables
        int tmp;
        void* tmpPtr;

        // Item path (copy to keep the original)
        char pathCpy[4096] = {0};

        // Basename of the item
        char* base;

        // Item size in bytes
        uint64_t size;

        // File descriptor for file and reading buffer
        int fd;
        unsigned char buf[BUFFERSIZE];

        // Read bytes at last read operation
        int readBytes;

        // Item name length for header
        unsigned int nameLen;

        // Get basename
        if (strcmp(".", path) == 0 || strcmp("./", path) == 0) {
                // Copy name of current directory instead to get the actual name
                tmpPtr = getcwd(pathCpy, 4096);
                if (tmpPtr == NULL) {
                        retVal = -1;
                        goto error;
                }
        } else {
                strncpy(pathCpy, path, 4096);
        }
        base = basename(pathCpy);

        // Calculate item name length
        nameLen = (strlen(base)) / 128;
        if (nameLen > 31) {
                (*header).item.nameLen = 31;
                pathCpy[4095] = 0;
        } else {
                (*header).item.nameLen = nameLen;
        }

        // Send header
        tmp = sendMultiple(socketID, &(*header).byte, 1, 0, MAX_TRIES_EINTR);
        if (tmp == -1) {
                perror("An error ocurred while sending the item header byte");
                retVal = -1;
                goto error;
        }

        // Send item name
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
                                // Sending file
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
int sendInner(int* socketID, char* path)
{
        // Item header for items directory
        union ItemHeader header;

        // Stuff to read directory contents
        DIR* dir;
        struct dirent* entry;

        // Function return value
        int retVal = 0;

        // Path to item
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

        // Initial memory allocation
        pathCurr = (char*) malloc((strlen(path) + 1 + strlen(entry->d_name) + 1) * sizeof(char));
        if (pathCurr == NULL) {
                fprintf(stderr, "An error ocurred while allocating memory for the path string.\n");
                retVal = -1;
                goto errorBeforeMalloc;
        }

        while (entry != NULL) {
                // Check for . and ..
                if (strncmp(entry->d_name, ".", 2) == 0 || strncmp(entry->d_name, "..", 3) == 0) {
                        // Skip
                        errno = 0;
                        entry = readdir(dir);
                        if (errno != 0) {
                                fprintf(stderr, "An error ocurred while reading the directory %s: %s\n", path, strerror(errno));
                                retVal = -1;
                                goto error;
                        }

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

                // Is last item?
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

                if (header.item.type == TYPE_DIRECTORY && header.item.emptyDirectory == 0) {
                        tmp = sendInner(socketID, pathCurr);
                        if (tmp == -1) {
                                retVal = -1;
                                goto error;
                        }
                }
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
ssize_t sendMultiple(int* sockfd, const void* buf, size_t len, int flags, int tries)
{
        ssize_t tmp;

        do {
                tmp = send(*sockfd, buf, len, flags);
                tries--;
        } while (tmp == -1 && tries > 0 && errno == EINTR);

        return tmp;
}

/**
 * Read data from a file with multiple tries in case of an interrupt
 */
ssize_t readMultiple(int fd, void* buf, size_t count, int tries)
{
        ssize_t tmp;

        do {
                tmp = read(fd, buf, count);
                tries--;
        } while (tmp == -1 && tries > 0 && errno == EINTR);

        return tmp;
}
