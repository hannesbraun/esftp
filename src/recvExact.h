/**
 * @file recvExact.h
 * @brief File contains the header for the recvExact function.
 * @author Hannes Braun
 * @date 15.05.2019
 */

#ifndef recvExact_h
#define recvExact_h

int recvExact(int socketID, void* buf, int bufLen, int flags);

#endif
