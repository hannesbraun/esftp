/**
 * Server (main) header
 */

#ifndef server_h
#define server_h

enum ShutdownState {
        noShutdown,
        friendlyShutdown,
        forceShutdown
};

extern volatile enum ShutdownState serverShutdownState;

#endif
