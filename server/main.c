#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#inlcude <unistd.h>

#define FALSE 0
#define TRUE 1

typedef struct ServerArguments_t {
    unsigned char ucArgumentsValid;
    short int siPort;
    char* pcFilePath;
} ServerArguments;

typedef struct ServerControl_t {
    unsigned char ucServerStop;
} ServerControl;

int main(int argc, char* argv[])
{
    ServerArguments sArguments;
    
    parseArguments(argc, argv, &sArguments);
    
    // Only start server if arguments are valid
    if (ucArgumentsValid == TRUE)
    {
        printf("Starting esftp server...\n");
    }
    
    return EXIT_SUCCESS;
}

/**
 * @fn void parseArguments(int argc, const char* argv[], ServerArguments* sArguments)
 * @brief Parses the given command line arguments and stores the parseed values in the given struct.
 * @param argc number of command line arguments
 * @param argv values of command line arguments
 * @param sArguments the struct to write the parsed information to
 * @return void
 * @author Hannes Braun
 * @date 12.05.2019
 *
 * Required format: ./esftp-server 31416 /path/to/file
 */
void parseArguments(int argc, const char* argv[], ServerArguments* sArguments)
{
    if (argc >= 3)
    {
        sArguments->ucArgumentsValid = TRUE;
        
        // Parse port number
        sArguments->siPort = atoi(argv[1]);
        if (sArguments->siPort == 0 || sArguments->siPort > 65535)
        {
            // Invalid input
            sArguments->ucArgumentsValid = FALSE;
            fprintf(stderr, "The given port number is not a valid number.\n");
        }
        else if (sArguments->siPort > 65536)
        {
            // Out of range, not a valid port number
            sArguments->ucArgumentsValid = FALSE;
            fprintf(stderr, "The given number %d is not a valid port number.\n", sArguments->siPort);
        }
        
        // Parse file path
        sArguments->pcFilePath = argv[2];
        if (access(sArguments->pcFilePath, R_OK) == -1)
        {
            // File doesn't exist or is not readable
            sArguments->ucArgumentsValid = FALSE;
            fprintf(stderr, "Error while trying to access the file %s:", sArguments->pcFilePath);
            perror(NULL);
        }
    }
    else
    {
        // Not enough arguments
        sArguments->ucArgumentsValid = FALSE;
    }
}
