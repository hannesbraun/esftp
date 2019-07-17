/**
 * @file main.c
 * @brief File contains the main function for the server as well as the parsing of the command line arguments.
 * @author Hannes Braun
 * @date 12.05.2019
 */

#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"
#include "serverConfig.h"

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
    ServerConfiguration sConfiguration;

    // Thread id of lobby
    pthread_t tidLobbyThread;

    // Char arrays for working with console input
    char acReadBuffer[100];
    char* acExitValue = "shutdown\n";
    
    // New sigaction for SIGPIPE
    struct sigaction newSigactionSigpipe;
    newSigactionSigpipe.sa_handler = SIG_IGN;

    // General purpose return value
    int iReturnValue;

    parseAndConfigure(argc, argv, &sConfiguration);

    if (sConfiguration.ucVersionFlag == 1)
    {
        printf(ESFTP_VERSION);
    }
    else if (sConfiguration.ucArgumentsValid == TRUE)
    {
            // Only start server if arguments are valid
            printf("Starting esftp server...\n");
        
            // Disable SIGPIPE
            sigaction(SIGPIPE, &newSigactionSigpipe, NULL);

            // Starting the lobby thread
            iReturnValue = pthread_create(&tidLobbyThread, NULL, lobby, &sConfiguration);
            if (iReturnValue != 0)
            {
                fprintf(stderr, "An error ocurred while starting the lobby. pthread_create returned %d", iReturnValue);
            }

            do
            {
                fgets(acReadBuffer, 100, stdin);
            } while (strcmp(acReadBuffer, acExitValue) != 0);
        
    }

    return EXIT_SUCCESS;
}

/**
 * @fn void parseAndConfigure(int argc, char* argv[], ServerConfiguration* psConfiguration)
 * @brief This function parses the given command line arguments and stores the parseed values in the given struct.
 * @param argc amount of command line arguments
 * @param argv values of command line arguments
 * @param psConfiguration the struct to write the parsed information to
 * @return void
 * @author Hannes Braun
 * @date 09.07.2019
 */
void parseAndConfigure(int argc, char* argv[], ServerConfiguration* psConfiguration)
{
    psConfiguration->ucArgumentsValid = TRUE;
    psConfiguration->ucVersionFlag = 0;
    psConfiguration->siPort = ESFTP_PORT;

    int iOptCode;
    int iOptionIndex;
    struct option asLongOptions[] =
    {
        {"version", no_argument, NULL, 1},
        {"port", required_argument, NULL, 'p'},
        {NULL, 0, NULL, 0}
    };

    while (1)
    {
        iOptCode = getopt_long(argc, argv, "p:", asLongOptions, &iOptionIndex);

        if (iOptCode == -1)
        {
            // No more options found
            break;
        }

        switch (iOptCode)
        {
            case 1:
                // Version
                psConfiguration->ucVersionFlag = 1;
                break;

            case 'p':
                // Port
                psConfiguration->siPort = atoi(optarg) % 65536;
                if (psConfiguration->siPort <= 0)
                {
                    // Invalid input
                    psConfiguration->ucArgumentsValid = FALSE;
                    fprintf(stderr, "The given port number is not valid.\n");
                }
                break;

            case '?':
                    break;
            default:
                break;
        }
    }

    if (optind < argc)
    {
        // Parse file path
        psConfiguration->pcFilePath = argv[optind];
        if (access(psConfiguration->pcFilePath, R_OK) == -1)
        {
            // File doesn't exist or is not readable
            psConfiguration->ucArgumentsValid = FALSE;
            fprintf(stderr, "Error while trying to access the file %s:", psConfiguration->pcFilePath);
            perror(NULL);
        }
    }
    else if (psConfiguration->ucVersionFlag == 0)
    {
        // Not enough arguments
        psConfiguration->ucArgumentsValid = FALSE;
        fprintf(stderr, "Not enough arguments given.\n");
        fprintf(stderr, "Usage: %s <options> <file path>\n", argv[0]);
    }
}
