/**
 * @file main.c
 * @brief File contains the main function for the client as well as the parseArguments function.
 * @author Hannes Braun
 * @date 16.06.2019
 */

#include <stdio.h>
#include <stdlib.h>

#include "client.h"

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
    ClientArguments sArguments;

    parseArguments(argc, argv, &sArguments);

    // Only start client if arguments are valid
    if (sArguments.ucArgumentsValid == TRUE)
    {
        connectAndReceive(&sArguments);
    }

    return EXIT_SUCCESS;
}

/**
 * @fn void parseArguments(int argc, char* argv[], ClientArguments* psArguments)
 * @brief This function parses the command line arguments.
 * @param argc the amount of command line arguments
 * @param argv the command line arguments
 * @param psArguments the struct to write the parsed data to
 * @return void
 * @author Hannes Braun
 * @date 16.06.2019
 */
void parseArguments(int argc, char* argv[], ClientArguments* psArguments)
{
    if (argc >= 3)
    {
        psArguments->ucArgumentsValid = TRUE;

        // Parse ip address
        psArguments->sIPAddress.s_addr = inet_addr(argv[1]);
        if ((int) psArguments->sIPAddress.s_addr == -1)
        {
            fprintf(stderr, "The given ip address doesn't seem to be valid.\n");
            psArguments->ucArgumentsValid = FALSE;
        }

        // Parse port number
        psArguments->siPort = atoi(argv[2]);
        if (psArguments->siPort == 0)
        {
            // Invalid input
            psArguments->ucArgumentsValid = FALSE;
            fprintf(stderr, "The given port number is not a valid number.\n");
        }
    }
    else
    {
        // Not enough arguments
        psArguments->ucArgumentsValid = FALSE;
        fprintf(stderr, "Not enough arguments given.\n");
        fprintf(stderr, "Usage: %s <ip address> <port number>\n", argv[0]);
    }
}
