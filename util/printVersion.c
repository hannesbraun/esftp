/**
 * @file printVersion.c
 * @brief File contains the printVersion function.
 * @author Hannes Braun
 * @date 22.07.2019
 */

#include "printVersion.h"

/**
 * @fn void printVersion(VersionOutput eVersionOutput)
 * @brief This function prints the version for the client/server.
 * @param eVersionOutput output server or client version
 * @return void
 * @author Hannes Braun
 * @date 22.07.2019
 */
void printVersion(VersionOutput eVersionOutput)
{
        if (eVersionOutput == server)
                printf("esftp-server");
        else if (eVersionOutput == client)
                printf("esftp-client");
        printf(" (esftp) ");

        printf(ESFTP_VERSION);

        putchar('\n');
}
