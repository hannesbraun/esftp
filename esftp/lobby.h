/**
 * Lobby header
 */

#ifndef lobby_h
#define lobby_h

struct LobbyConfig {
        char** items;
        short int port;
        unsigned char printVersion: 1;
        unsigned char argumentsValid: 1;
};

void lobby(LobbyConfig* config);

#endif
