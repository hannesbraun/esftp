/**
 * Main module for the server
 */

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "printVersion.h"
#include "server.h"
#include "config.h"
#include "lobby.h"

int main(int argc, char* argv[])
{
        // General purpose return value
        int retVal;

        // Return value (program exit)
        int retValExit = EXIT_FAILURE;

        LobbyConfig config;

        //VersionOutput versionOutput = server;

        // New sigaction for SIGPIPE
        struct sigaction newSigactionSigpipe;
        newSigactionSigpipe.sa_handler = SIG_IGN;

        // New sigaction for SIGINT
        struct sigaction newSigactionSigint;
        newSigactionSigint.sa_handler = &sigintHandler;

        parseAndConfigure(argc, argv, &config);

        //if (config.ucVersionFlag == 1)
        //        printVersion(eVersionOutput);
        if (config.argumentsValid == 1) {
                // Disable SIGPIPE
                sigaction(SIGPIPE, &newSigactionSigpipe, NULL);

                // Initialize SIGINT handler
                serverShutdownState = noShutdown;
                retVal = sigaction(SIGINT, &newSigactionSigint, NULL);
                if (retVal == -1) {
                        perror("An error ocurred while changing the SIGINT action");
                        goto error;
                }

                // Execute the lobby
                lobby(&config);

                retValExit = EXIT_SUCCESS;
        }

error:

        return retValExit;
}

/**
 * This function parses the given command line arguments.
 * The parsed information will be written into config.
 */
void parseAndConfigure(int argc, char** argv, LobbyConfig* config)
{
        // Counting variable for reading the paths
        long int itemNum = 0;

        // Defaults
        config->argumentsValid = TRUE;
        config->printVersion = 0;
        config->port = ESFTP_PORT;

        int optCode;
        int optionIndex;
        struct option longOptions[] = {
                {"version", no_argument, NULL, 1},
                {"port", required_argument, NULL, 'p'},
                {NULL, 0, NULL, 0}
        };

        while (1) {
                optCode = getopt_long(argc, argv, "p:", longOptions, &optionIndex);

                if (optCode == -1) {
                        // No more options found
                        break;
                }

                switch (optCode) {
                        case 1:
                                // Version
                                config->printVersion = 1;
                                break;

                        case 'p':
                                // Port
                                config->port = atoi(optarg) % 65536;
                                if (config->port <= 0) {
                                        // Invalid input
                                        config->argumentsValid = 0;
                                        fprintf(stderr, "The given port number is not valid.\n");
                                }
                                break;

                        case '?':
                        default:
                                break;
                }
        }

        if (optind < argc) {
                do {
                        // Store paths
                        config->items[itemNum] = argv[optind];
                        optind++;
                        itemNum++;
                }  while (optind < argc);
        } else if (config->printVersion == 0) {
                // Not enough arguments
                config->argumentsValid = 0;
                fprintf(stderr, "Not enough arguments given.\n");
                fprintf(stderr, "Usage: %s <options> <items to send>\n", argv[0]);
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
