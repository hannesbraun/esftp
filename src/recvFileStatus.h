#ifndef recvFileStatus_h
#define recvFileStatus_h

#include <inttypes.h>
#include <sys/time.h>

void printStatus(uint64_t* byteProgress, struct timeval* timeProgress, uint64_t size, unsigned char erase);

#endif
