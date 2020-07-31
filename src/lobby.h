/**
 * Lobby header
 */

#ifndef lobby_h
#define lobby_h

struct LobbyConfig {
        char** items;
        unsigned long int itemsLen;
        short int port;
        unsigned char printVersion: 1;
};

int lobby(struct LobbyConfig* config);

#endif
