/**
 * @file main.c
 * @brief File contains the main function for the client as well as the parseArguments function.
 * @author Hannes Braun
 * @date 16.06.2019
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "client.h"
#include "commons.h"
#include "printVersion.h"

/**
 * @fn int main(int argc, char* argv[])
 * @brief This is the main function for the client.
 * @param argc the amount of command line arguments
 * @param argv the command line arguments
 * @return int EXIT_SUCCESS in case of a successful exit
 * @author Hannes Braun
 * @date 16.06.2019
 */
int main(int argc, char* argv[])
{
        int retVal = EXIT_SUCCESS;
        int tmp;

        struct Config config;

        tmp = parseAndConfigure(argc, argv, &config);

        if (tmp == -1) {
                retVal = EXIT_FAILURE;
                goto error;
        }

        if (config.printVersion == 1) {
                printVersion(client);
        } else {
                tmp = connectAndReceive(&config);
                if (tmp == -1) {
                        retVal = EXIT_FAILURE;
                }
        }

error:

        return retVal;
}

/**
 * @fn void parseAndConfigure(int argc, char* argv[], ClientConfiguration* psConfiguration)
 * @brief This function parses the given command line arguments and stores the parseed values in the given struct.
 * @param argc amount of command line arguments
 * @param argv values of command line arguments
 * @param psConfiguration the struct to write the parsed information to
 * @return void
 * @author Hannes Braun
 * @date 09.07.2019
 */
int parseAndConfigure(int argc, char* argv[], struct Config* config)
{
        int retVal = 0;
        int tmp;

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

                optCode = getopt_long(argc, argv, "o:p:", longOptions, &optionIndex);

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
                // Parse ip address
                tmp = inet_aton(argv[optind], &(config->addr));
                if (tmp == 0) {
                        fprintf(stderr, "The given ip address is not valid.\n");
                        retVal = -1;
                }
        } else if (config->printVersion == 0) {
                // Not enough arguments
                retVal = -1;
                fprintf(stderr, "Not enough arguments given.\n");
                fprintf(stderr, "Usage: %s <options> <ip address>\n", argv[0]);
        }

        return retVal;
}
