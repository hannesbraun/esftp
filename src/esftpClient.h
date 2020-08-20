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

#ifndef esftpClient_h
#define esftpClient_h

#include <arpa/inet.h>

#define RECVBUFFERSIZE 4096

struct ClientConfig {
        struct in_addr addr;
        short int port;
        unsigned char printHelp: 1;
        unsigned char printVersion: 1;
};

int connectAndReceive(struct ClientConfig* config);

#endif
