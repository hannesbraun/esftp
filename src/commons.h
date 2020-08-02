/**
 * Shared stuff between the client and the server
 */

#ifndef commons_h
#define commons_h

#define ESFTP_VERSION "0.3"

// Default port
#define ESFTP_PORT 6719

union ItemHeader {
        struct {
                unsigned char type: 1;
                unsigned char lastItem: 1;
                unsigned char emptyDirectory: 1;
                unsigned char nameLen: 5;
        } item;
        unsigned char byte;
};
#define TYPE_FILE 0
#define TYPE_DIRECTORY 1

#endif
