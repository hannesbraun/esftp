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

#include <stdio.h>

#include "commons.h"
#include "printVersion.h"

/**
 * This function prints the version for the client/server.
 */
void printVersion(enum VersionOutput versionOutput)
{
        if (versionOutput == server) {
                printf("esftp-server (esftp) " ESFTP_VERSION "\n");
        } else if (versionOutput == client) {
                printf("esftp-client (esftp) " ESFTP_VERSION "\n");
        }
}
