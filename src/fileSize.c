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

#define _FILE_OFFSET_BITS 64

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "fileSize.h"

/**
 * This function calculates the size of the given file.
 * In the case of an error, -1 will be returned (else 0).
 */
int calculateFileSize(char* path, uint64_t* size)
{
        // File descriptor of the file to get the size of
        int fd;

        // File size
        int64_t readSize;

        // General purpose value
        int tmp;
        int retVal = 0;

        // Open file
        fd = open(path, O_RDONLY);
        if (fd == -1) {
                perror("An error ocurred while opening the file for calculating file size");
                retVal = -1;
                goto error;
        }

        // Get file size
        readSize = lseek(fd, 0, SEEK_END);
        if (readSize == -1) {
                perror("An error ocurred while getting the file size via lseek");
                retVal = -1;
        } else {
                *size = readSize;
        }

error:
        // Close file
        tmp = close(fd);
        if (tmp == -1) {
                perror("An error ocurred while closing the file after getting its size");
                retVal = -1;
        }

        return retVal;
}
