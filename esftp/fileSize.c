/**
 * @file fileSize.c
 * @brief File contains the fileSize function.
 * @author Hannes Braun
 * @date 16.05.2019
 */

#define _FILE_OFFSET_BITS 64

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "fileSize.h"

/**
 * This function calculates the size of the given file. In the case of an error
 */
int64_t calculateFileSize(char* path)
{
        // File descriptor of the file to get the size of
        int fd;

        // File size
        int64_t size;

        // General purpose value
        int tmp;

        // Open file
        fd = open(path, O_RDONLY);
        if (fd == -1) {
                perror("An error ocurred while opening the file for calculating file size");
                size = -1;
                goto error;
        }

        // Get file size
        size = lseek(fd, 0, SEEK_END);
        if (size == -1) {
                perror("An error ocurred while getting the file size via lseek");
        }

error:
        // Close file
        tmp = close(fd);
        if (tmp == -1) {
                perror("An error ocurred while closing the file after getting its size");
                size = -1;
        }

        return size;
}
