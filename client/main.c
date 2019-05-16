#include <stdio.h>
#include <stdlib.h>

#include "client.h"

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
