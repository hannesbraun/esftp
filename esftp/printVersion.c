/**
 * @file printVersion.c
 * @brief File contains the printVersion function.
 * @author Hannes Braun
 * @date 22.07.2019
 */

#include <stdio.h>
#include "printVersion.h"

/**
 * @fn void printVersion(VersionOutput eVersionOutput)
 * @brief This function prints the version for the client/server.
 * @param eVersionOutput output server or client version
 * @return void
 * @author Hannes Braun
 * @date 22.07.2019
 */
void printVersion(enum VersionOutput versionOutput)
{
        if (versionOutput == server) {
                printf("esftp-server");
        } else if (versionOutput == client) {
                printf("esftp-client");
        }
        printf(" (esftp) ");

        printf(ESFTP_VERSION);

        putchar('\n');
}
