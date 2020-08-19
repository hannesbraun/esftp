#include <stdio.h>

#include "commons.h"
#include "printVersion.h"

/**
 * This function prints the version for the client/server.
 */
void printVersion(enum VersionOutput versionOutput)
{
        if (versionOutput == server) {
                printf("esftp-server (esftp) " ESFTP_VERSION "\n");
        } else if (versionOutput == client) {
                printf("esftp-client (esftp) " ESFTP_VERSION "\n");
        }
}
