/******************************************************************************
 * Copyright 2019-2020 Hannes Braun
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 ******************************************************************************/

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
