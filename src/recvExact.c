#include <sys/socket.h>

/**
 * This function receives an exact amount of bytes over TCP.
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
