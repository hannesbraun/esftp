#include <stdlib.h>
#include <string.h>

#define FALSE 0
#define TRUE 1

typedef struct ServerArguments_t {
    unsigned char ucArgumentsValid;
    short int siPort;
    char* pcFilePath;
} ServerArguments;

typedef struct ServerControl_t {
    unsigned char ucServerStop;
} ServerControl;

int main(int argc, char* argv[])
{
    ServerArguments sArguments;
    
    parseArguments(argc, argv, &sArguments)
    
    // Only start server if arguments are valid
    if (ucArgumentsValid == TRUE)
    {
        printf("Starting esftp server...\n");
    }
    
    return EXIT_SUCCESS;
}

void parseArguments(int argc, const char* argv[], ServerArguments* sArguments)
{
    if (argc > 1)
    {
        sArguments->ucArgumentsValid = TRUE;
        sArguments->siPort = 31416;
        sArguments->pcFilePath = argv[1];
    }
    else
    {
        sArguments->ucArgumentsValid = FALSE;
    }
}
