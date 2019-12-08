/**
 * @file fileSize.c
 * @brief File contains the fileSize function.
 * @author Hannes Braun
 * @date 16.05.2019
 */

#define _FILE_OFFSET_BITS 64

#include "fileSize.h"

/**
 * @fn unsigned long int calculateFileSize(char* pcFilePath)
 * @brief This function calculates the size of the given file.
 * @param pcFilePath the file path as a char array
 * @return unsigned long int the file size in bytes
 * @author Hannes Braun
 * @date 16.05.2019
 */
int64_t calculateFileSize(char* pcFilePath)
{
        // File descriptor of the file to get the size of
        int iFileDescriptor;

        // File size
        int64_t i64FileSize;

        // General purpose return value
        int iReturnValue;

        // Open file
        iFileDescriptor = open(pcFilePath, O_RDONLY);
        if (iFileDescriptor == -1)
                perror("An error ocurred while opening the file for calculating file size");

        // Get file size
        i64FileSize = lseek(iFileDescriptor, 0, SEEK_END);
        if (i64FileSize == -1)
                perror("An error ocurred while getting the file size via lseek");

        // Close file
        iReturnValue = close(iFileDescriptor);
        if (iReturnValue == -1)
                perror("An error ocurred while closing the file after getting its size");

        return i64FileSize;
}
