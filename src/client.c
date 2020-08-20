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
 * Main client module
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "commons.h"
#include "esftpClient.h"
#include "printVersion.h"

int parseAndConfigure(int argc, char* argv[], struct ClientConfig* config);
void printHelp(void);

/**
 * Main function for client
 */
int main(int argc, char* argv[])
{
        // Process return value
        int retVal = EXIT_SUCCESS;

        int tmp;

        struct ClientConfig config;

        tmp = parseAndConfigure(argc, argv, &config);
        if (tmp == -1) {
                retVal = EXIT_FAILURE;
                goto error;
        }

        if (config.printHelp == 1) {
                printHelp();
        } else if (config.printVersion == 1) {
                // Print client version and exit
                printVersion(client);
        } else {
                // Connect to server and receive data
                tmp = connectAndReceive(&config);
                if (tmp == -1) {
                        retVal = EXIT_FAILURE;
                        goto error;
                }
        }

error:
        return retVal;
}

/**
 * This function parses the given command line arguments and stores the parseed values in the given struct.
 */
int parseAndConfigure(int argc, char* argv[], struct ClientConfig* config)
{
        int retVal = 0;
        int tmp;

        config->printHelp = 0;
        config->printVersion = 0;
        config->port = ESFTP_PORT;

        int optCode;
        int optionIndex;
        struct option longOptions[] = {
                {"version", no_argument, NULL, 1},
                {"help", no_argument, NULL, 'h'},
                {"port", required_argument, NULL, 'p'},
                {NULL, 0, NULL, 0}
        };

        while (1) {
                // Get next option
                optCode = getopt_long(argc, argv, "p:h", longOptions, &optionIndex);
                if (optCode == -1) {
                        // No more options found
                        break;
                }

                switch (optCode) {
                        case 1:
                                // Version will be printed
                                config->printVersion = 1;
                                break;
                        case 'h':
                                // Help will be printed
                                config->printHelp = 1;
                                break;
                        case 'p':
                                // Port
                                config->port = atoi(optarg) % 65536;
                                if (config->port <= 0) {
                                        // Invalid input
                                        retVal = -1;
                                        fprintf(stderr, "The given port number is not valid.\n");
                                }
                                break;

                        case '?': // Error message already printed by getopt_long
                        default:
                                break;
                }

        }

        if (optind < argc) {
                // Parse ip address
                tmp = inet_aton(argv[optind], &(config->addr));
                if (tmp == 0) {
                        fprintf(stderr, "The given ip address is not valid.\n");
                        retVal = -1;
                }
        } else if (config->printVersion == 0 && config->printHelp == 0) {
                // Not enough arguments
                retVal = -1;
                fprintf(stderr, "Not enough arguments given.\n");
                fprintf(stderr, "Usage: %s <options> <server>\n", argv[0]);
        }

        return retVal;
}

void printHelp(void)
{
        printf("esftp-client\n\nUsage: esftp-client [-p PORT] server\n\nesftp-client receives one or multiple files and/or directories from a server over the ESFTP protocol. "
               "The received top-level files and directories will be stored in the current working directory. "
               "The given server has to be an ip address. Domain names and hostnames are not supported.\n\n"
               "Options:\n"
               "    -p, --port <PORT>: Sets the port of the server to connect to.\n"
               "        Defaults to 6719.\n"
               "    -h, --help: Prints a help text including a list of available options.\n"
               "    --version: Prints the version of esftp-client.\n\n");
}
