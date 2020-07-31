/**
 * @file printVersion.h
 * @brief File contains the header for the printVersion function.
 * @author Hannes Braun
 * @date 22.07.2019
 */

#ifndef printVersion_h
#define printVersion_h

#define ESFTP_VERSION "0.3"

enum VersionOutput {
        server,
        client
};

void printVersion(enum VersionOutput versionOutput);

#endif
