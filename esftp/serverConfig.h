/**
 * General configuation properties for the server
 */

#ifndef serverconfig_h
#define serverconfig_h

// Backlog size
#define BACKLOGSIZE 42

// Block size for reading the data out of a file
#define BUFFERSIZE 4096

// Max. amount of items
#define MAX_ITEMS 256

// Max. tries of sending when an interrupt occurs
#define MAX_TRIES_EINTR 8

#endif