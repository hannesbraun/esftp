/**
 * @file printVersion.h
 * @brief File contains the header for the printVersion function.
 * @author Hannes Braun
 * @date 22.07.2019
 */

#ifndef printVersion_h
#define printVersion_h

#include <stdio.h>

#define ESFTP_VERSION "0.2"

typedef enum VersionOutput_t
{
    server,
    client
} VersionOutput;

void printVersion(VersionOutput eVersionOutput);

#endif
