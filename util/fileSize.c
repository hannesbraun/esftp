/**
 * @file fileSize.c
 * @brief File contains the fileSize function.
 * @author Hannes Braun
 * @date 16.05.2019
 */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

/**
 * @fn unsigned long int calculateFileSize(char* pcFilePath)
 * @brief This function calculates the size of the given file.
 * @param pcFilePath the file path as a char array
 * @return unsigned long int the file size in bytes
 * @author Hannes Braun
 * @date 16.05.2019
 */
unsigned long int calculateFileSize(char* pcFilePath)
{
    int iFileDescriptor;
    unsigned int uliFileSize;
    
    // General purpose return value
    int iReturnValue;
    
    // Open file
    iFileDescriptor = open(pcFilePath, O_RDONLY);
    if (iFileDescriptor == -1)
    {
        perror("An error ocurred while opening the file for calculating file size");
    }
    
    // Get file size
    uliFileSize = lseek(iFileDescriptor, 0, SEEK_END);
    if (uliFileSize == -1)
    {
        perror("An error ocurred while getting the file size via lseek");
    }
    
    // Close file
    iReturnValue = close(iFileDescriptor);
    if (iReturnValue == -1)
    {
        perror("An error ocurred while closing the file after getting its size");
    }
    
    return uliFileSize;
}
