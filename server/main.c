/**
 * @file main.c
 * @brief File contains the main function for the server as well as the parsing of the command line arguments.
 * @author Hannes Braun
 * @date 12.05.2019
 */

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../util/printVersion.h"
#include "server.h"
#include "serverConfig.h"

/**
 * @fn int main(int argc, char* argv[])
 * @brief This is the main function for the server.
 * @param argc amount of command line arguments
 * @param argv values of command line arguments
 * @return int exit value of the process
 * @author Hannes Braun
 * @date 21.07.2019
 */
int main(int argc, char* argv[])
{
        // General purpose return value
        int iReturnValue;

        ServerConfiguration sConfiguration;

        VersionOutput eVersionOutput = server;

        // New sigaction for SIGPIPE
        struct sigaction newSigactionSigpipe;
        newSigactionSigpipe.sa_handler = SIG_IGN;

        // New sigaction for SIGINT
        struct sigaction newSigactionSigint;
        newSigactionSigint.sa_handler = &sigintHandler;

        parseAndConfigure(argc, argv, &sConfiguration);

        if (sConfiguration.ucVersionFlag == 1)
                printVersion(eVersionOutput);
        else if (sConfiguration.ucArgumentsValid == TRUE) {
                // Only start server if arguments are valid
                printf("Starting esftp server...\n");

                // Disable SIGPIPE
                sigaction(SIGPIPE, &newSigactionSigpipe, NULL);

                // Initialize SIGINT handler
                serverShutdownState = noShutdown;
                iReturnValue = sigaction(SIGINT, &newSigactionSigint, NULL);
                if (iReturnValue == -1) {
                        perror("An error ocurred while changing the SIGINT action");
                        goto error;
                }

                // Executing the lobby
                lobby(&sConfiguration);
        }

error:

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
        struct option asLongOptions[] = {
                {"version", no_argument, NULL, 1},
                {"port", required_argument, NULL, 'p'},
                {NULL, 0, NULL, 0}
        };

        while (1) {
                iOptCode = getopt_long(argc, argv, "p:", asLongOptions, &iOptionIndex);

                if (iOptCode == -1) {
                        // No more options found
                        break;
                }

                switch (iOptCode) {
                        case 1:
                                // Version
                                psConfiguration->ucVersionFlag = 1;
                                break;

                        case 'p':
                                // Port
                                psConfiguration->siPort = atoi(optarg) % 65536;
                                if (psConfiguration->siPort <= 0) {
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

        if (optind < argc) {
                // Parse file path
                psConfiguration->pcFilePath = argv[optind];
                if (access(psConfiguration->pcFilePath, R_OK) == -1) {
                        // File doesn't exist or is not readable
                        psConfiguration->ucArgumentsValid = FALSE;
                        fprintf(stderr, "Error while trying to access the file %s:", psConfiguration->pcFilePath);
                        perror(NULL);
                }
        } else if (psConfiguration->ucVersionFlag == 0) {
                // Not enough arguments
                psConfiguration->ucArgumentsValid = FALSE;
                fprintf(stderr, "Not enough arguments given.\n");
                fprintf(stderr, "Usage: %s <options> <file path>\n", argv[0]);
        }
}

void sigintHandler(int iSignum)
{
        if (serverShutdownState == noShutdown)
                serverShutdownState = friendlyShutdown;
        else if (serverShutdownState == friendlyShutdown)
                serverShutdownState = forceShutdown;
        else if (serverShutdownState == forceShutdown)
                exit(-1);
}
