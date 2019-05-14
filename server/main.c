/**
 * @file main.c
 * @brief File contains the main function for the server as well as the parsing of the command line arguments.
 * @author Hannes Braun
 * @date 12.05.2019
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"

/**
 * @fn int main(int argc, char* argv[])
 * @brief This is the main function for the server.
 * @param argc amount of command line arguments
 * @param argv values of command line arguments
 * @return int exit value of the process
 * @author Hannes Braun
 * @date 12.05.2019
 */
int main(int argc, char* argv[])
{
    ServerArguments sArguments;
    pthread_t tidLobbyThread;
    
    char acReadBuffer[100];
    char* acExitValue = "shutdown\n";
    
    // General purpose return value
    int iReturnValue;
    
    parseArguments(argc, argv, &sArguments);
    
    // Only start server if arguments are valid
    if (sArguments.ucArgumentsValid == TRUE)
    {
        printf("Starting esftp server...\n");
        
        // Starting the lobby thread
        iReturnValue = pthread_create(&tidLobbyThread, NULL, lobby, &sArguments);
        if (iReturnValue != 0)
        {
            fprintf(stderr, "An error ocurred while starting the lobby. pthread_create returned %d", iReturnValue);
        }
        
        do {
            printf(">");
            fgets(acReadBuffer, 100, stdin);
        } while (strcmp(acReadBuffer, acExitValue) != 0);
    }
    
    return EXIT_SUCCESS;
}

/**
 * @fn void parseArguments(int argc, char* argv[], ServerArguments* sArguments)
 * @brief Parses the given command line arguments and stores the parseed values in the given struct.
 * @param argc amount of command line arguments
 * @param argv values of command line arguments
 * @param sArguments the struct to write the parsed information to
 * @return void
 * @author Hannes Braun
 * @date 12.05.2019
 *
 * Required format: ./esftp-server 31416 /path/to/file
 */
void parseArguments(int argc, char* argv[], ServerArguments* psArguments)
{
    if (argc >= 3)
    {
        psArguments->ucArgumentsValid = TRUE;
        
        // Parse port number
        psArguments->siPort = atoi(argv[1]);
        if (psArguments->siPort == 0)
        {
            // Invalid input
            psArguments->ucArgumentsValid = FALSE;
            fprintf(stderr, "The given port number is not a valid number.\n");
        }
        
        // Parse file path
        psArguments->pcFilePath = argv[2];
        if (access(psArguments->pcFilePath, R_OK) == -1)
        {
            // File doesn't exist or is not readable
            psArguments->ucArgumentsValid = FALSE;
            fprintf(stderr, "Error while trying to access the file %s:", psArguments->pcFilePath);
            perror(NULL);
        }
    }
    else
    {
        // Not enough arguments
        psArguments->ucArgumentsValid = FALSE;
        fprintf(stderr, "Not enough arguments given.\n");
        fprintf(stderr, "Usage: %s <port number> <file path>\n", argv[0]);
    }
}
