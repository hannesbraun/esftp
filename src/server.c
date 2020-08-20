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

int parseAndConfigure(int argc, char** argv, struct LobbyConfig* config);
void sigintHandler(int signum);
void printHelp(void);

/**
 * The main function calls a function to parse the command line arguments and executes the lobby afterwards.
 * It is also responsible to change the handlers for signals.
 */
int main(int argc, char** argv)
{
        // General purpose return variable
        int tmp;

        // Process return value
        int retVal = EXIT_SUCCESS;

        // Paths to items to send
        char* items[MAX_ITEMS];

        // Server configuration
        struct LobbyConfig config;
        config.items = items;
        config.itemsLen = 0;

        // Ignore SIGPIPE (to avoid killing the process if a connection breaks)
        struct sigaction newSigactionSigpipe;
        newSigactionSigpipe.sa_handler = SIG_IGN;

        // Better shutdown handling on SIGINT
        struct sigaction newSigactionSigint;
        newSigactionSigint.sa_handler = &sigintHandler;

        tmp = parseAndConfigure(argc, argv, &config);

        if (tmp == 0) {
                if (config.printHelp == 1) {
                        // Only print the help and exit
                        printHelp();
                        goto end;
                } else if (config.printVersion == 1) {
                        // Only print the version and exit
                        printVersion(server);
                        goto end;
                }

                // Set SIGPIPE handler
                tmp = sigaction(SIGPIPE, &newSigactionSigpipe, NULL);
                if (tmp == -1) {
                        perror("An error ocurred while disabling SIGPIPE");
                        retVal = EXIT_FAILURE;
                        goto error;
                }

                // Set SIGINT handler
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

end:
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
        // General purpose return value
        int tmp;

        // Return value indicating if the arguments are valid
        int retVal = 0;

        // Defaults
        config->printVersion = 0;
        config->port = ESFTP_PORT;

        int optCode;
        int optionIndex;

        // Available options
        struct option longOptions[] = {
                {"version", no_argument, NULL, 1},
                {"help", no_argument, NULL, 'h'},
                {"port", required_argument, NULL, 'p'},
                {NULL, 0, NULL, 0}
        };

        while (1) {
                // Parse next option
                optCode = getopt_long(argc, argv, "p:h", longOptions, &optionIndex);

                if (optCode == -1) {
                        // No more options found
                        break;
                }

                switch (optCode) {
                        case 1:
                                // Print version
                                config->printVersion = 1;
                                break;
                        case 'h':
                                // Help will be printed
                                config->printHelp = 1;
                                break;

                        case 'p':
                                // Set port
                                tmp = atoi(optarg) % 65536;
                                if (tmp <= 0) {
                                        // Invalid input
                                        retVal = -1;
                                        fprintf(stderr, "The given port number is not valid.\n");
                                } else {
                                        config->port = tmp;
                                }
                                break;

                        case '?':
                        default:
                                break;
                }
        }

        if (optind < argc) {
                // Store paths/items
                do {
                        config->items[config->itemsLen] = argv[optind];
                        optind++;
                        config->itemsLen++;
                        if (config->itemsLen >= MAX_ITEMS) {
                                retVal = -1;
                                fprintf(stderr, "Too much items. Only %d itmes are allowed.\n", MAX_ITEMS);
                                break;
                        }
                }  while (optind < argc);
        } else if (config->printVersion == 0 && config->printHelp == 0) {
                // Not enough arguments
                retVal = -1;
                fprintf(stderr, "Not enough arguments given.\n");
                fprintf(stderr, "Usage: %s <options> <items to send>\n", argv[0]);
        }

        return retVal;
}

/**
 * Signal handler for SIGINT
 * Modifies the server shutdown state trying to initiate a friendly shutdown
 */
void sigintHandler(int signum)
{
        // Suppress unused parameter warning
        (void) signum;

        if (serverShutdownState == noShutdown) {
                serverShutdownState = friendlyShutdown;
        } else if (serverShutdownState == friendlyShutdown) {
                serverShutdownState = forceShutdown;
        } else if (serverShutdownState == forceShutdown) {
                exit(EXIT_FAILURE);
        }
}

void printHelp(void)
{
        printf("esftp-server\n\nUsage: esftp-server [-p PORT] item ...\n\nesftp-server serves one or multiple files and/or directories over the ESFTP protocol. "
               "With -p or --port, the port on which the files and directories are served can be specified (defaults to 6719).\n\n"
               "Options:\n"
               "    -p, --port <PORT>: Sets the port on which to serve the items.\n"
               "        Defaults to 6719.\n"
               "    -h, --help: Prints a help text including a list of available options.\n"
               "    --version: Prints the version of esftp-server.\n\n");
}
