/**
 * Main module for the server
 */

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "commons.h"
#include "printVersion.h"
#include "server.h"
#include "serverConfig.h"
#include "lobby.h"

int parseAndConfigure(int argc, char* argv[], struct LobbyConfig* config);

int main(int argc, char* argv[])
{
        // General purpose return value
        int retVal = EXIT_SUCCESS;
        int tmp;

        struct LobbyConfig config;
        char* items[MAX_ITEMS];
        config.items = items;

        // New sigaction for SIGPIPE
        struct sigaction newSigactionSigpipe;
        newSigactionSigpipe.sa_handler = SIG_IGN;

        // New sigaction for SIGINT
        struct sigaction newSigactionSigint;
        newSigactionSigint.sa_handler = &sigintHandler;

        tmp = parseAndConfigure(argc, argv, &config);

        if (tmp == 0) {
                // Disable SIGPIPE (no need to terminate the process)
                tmp = sigaction(SIGPIPE, &newSigactionSigpipe, NULL);
                if (tmp == -1) {
                        perror("An error ocurred while disabling SIGPIPE");
                        retVal = EXIT_FAILURE;
                        goto error;
                }

                // Initialize SIGINT handler
                serverShutdownState = noShutdown;
                tmp = sigaction(SIGINT, &newSigactionSigint, NULL);
                if (tmp == -1) {
                        perror("An error ocurred while changing the SIGINT action");
                        retVal = EXIT_FAILURE;
                        goto error;
                }

                // Execute the lobby
                tmp = lobby(&config);
                if (tmp != 0) {
                        retVal = EXIT_FAILURE;
                }

        }

error:
        return retVal;
}

/**
 * This function parses the given command line arguments.
 * The parsed information will be written into config.
 * If the arguments are invalid, -1 will be returned. Else 0 will be returned.
 */
int parseAndConfigure(int argc, char** argv, struct LobbyConfig* config)
{
        // Return value indicating if the arguments are valid
        int retVal = 0;

        // Defaults
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
                                // Print version
                                config->printVersion = 1;
                                break;

                        case 'p':
                                // Set port
                                config->port = atoi(optarg) % 65536;
                                if (config->port <= 0) {
                                        // Invalid input
                                        retVal = -1;
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
                        // Store paths/items
                        config->items[config->itemsLen] = argv[optind];
                        optind++;
                        config->itemsLen++;
                        if (config->itemsLen >= MAX_ITEMS) {
                                retVal = -1;
                                fprintf(stderr, "Too much items. Only %d itmes are allowed.\n", MAX_ITEMS);
                                break;
                        }
                }  while (optind < argc);
        } else if (config->printVersion == 0) {
                // Not enough arguments
                retVal = -1;
                fprintf(stderr, "Not enough arguments given.\n");
                fprintf(stderr, "Usage: %s <options> <items to send>\n", argv[0]);
        }

        return retVal;
}

void sigintHandler(int signum)
{
        if (serverShutdownState == noShutdown) {
                serverShutdownState = friendlyShutdown;
        } else if (serverShutdownState == friendlyShutdown) {
                serverShutdownState = forceShutdown;
        } else if (serverShutdownState == forceShutdown) {
                exit(EXIT_FAILURE);
        }
}
