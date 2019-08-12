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
#include "../util/printVersion.h"

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
    int iApplicationReturn = EXIT_SUCCESS;

    ClientConfiguration sConfiguration;

    VersionOutput eVersionOutput = client;

    parseAndConfigure(argc, argv, &sConfiguration);

    if (sConfiguration.ucVersionFlag == 1)
    {
        printVersion(eVersionOutput);
    }
    else if (sConfiguration.ucArgumentsValid == TRUE)
    {
        // Only start client if arguments are valid
        iApplicationReturn = connectAndReceive(&sConfiguration);
    }

    return iApplicationReturn;
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
void parseAndConfigure(int argc, char* argv[], ClientConfiguration* psConfiguration)
{
    psConfiguration->ucArgumentsValid = TRUE;
    psConfiguration->pcOutputFileName = NULL;
    psConfiguration->ucVersionFlag = 0;
    psConfiguration->siPort = ESFTP_PORT;

    int iOptCode;
    int iOptionIndex;
    struct option asLongOptions[] =
    {
        {"version", no_argument, NULL, 1},
        {"port", required_argument, NULL, 'p'},
        {"output", required_argument, NULL, 'o'},
        {NULL, 0, NULL, 0}
    };

    while (1)
    {

        iOptCode = getopt_long(argc, argv, "o:p:", asLongOptions, &iOptionIndex);

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

            case 'o':
                // Output
                psConfiguration->pcOutputFileName = optarg;
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
            default:
                break;
        }

    }

    if (optind < argc)
    {
        // Parse ip address
        psConfiguration->sIPAddress.s_addr = inet_addr(argv[optind]);
        if ((int) psConfiguration->sIPAddress.s_addr == -1)
        {
            fprintf(stderr, "The given ip address doesn't seem to be valid.\n");
            psConfiguration->ucArgumentsValid = FALSE;
        }
    }
    else if (psConfiguration->ucVersionFlag == 0)
    {
        // Not enough arguments
        psConfiguration->ucArgumentsValid = FALSE;
        fprintf(stderr, "Not enough arguments given.\n");
        fprintf(stderr, "Usage: %s <options> <ip address>\n", argv[0]);
    }
}
