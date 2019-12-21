/**
 * @file recvExact.c
 * @brief File contains the recvExact function to receive an exact amount of bytes (TCP socket).
 * @author Hannes Braun
 * @date 15.05.2019
 */

#include <sys/socket.h>

/**
 * @fn int recvExact(int iSocketID, void* pvBuffer, int iBuflen, int iFlags)
 * @brief This function receives an exact amount of bytes over TCP.
 * @param iSocketID the file descriptor of the socket
 * @param pvBuffer the buffer to write the received bytes to
 * @param iBuflen the length of the buffer (this is the amount of bytes that will be received)
 * @param iFlags the flags to pass to recv
 * @return int the amount of received bytes or -1 in case of an error
 * @author Hannes Braun
 * @date 15.05.2019
 */
int recvExact(int socketID, void* buf, int bufLen, int flags)
{
        // Initialize amount of received bytes with zero
        int current = 0;
        int total = 0;

        while (bufLen > total) {
                // Receive operation
                current = recv(socketID, buf + total, bufLen - total, flags);
                if (current == -1) {
                        // Error
                        return -1;
                }

                // Update total
                total = total + current;
        }

        return total;
}
