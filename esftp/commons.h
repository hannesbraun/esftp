/**
 * Shared stuff between the client and the server
 */

#ifndef commons_h
#define commons_h

// Default port
#define ESFTP_PORT 6719

union ItemHeader {
        struct {
                unsigned char type: 1;
                unsigned char lastItem: 1;
                unsigned char emptyFolder: 1;
                unsigned char unusedBits: 5;
        } item;
        unsigned char byte;
};
#define TYPE_FILE 0
#define TYPE_FOLDER 1

#endif
