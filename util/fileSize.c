#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

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
    
    return uliFileSize;
}
