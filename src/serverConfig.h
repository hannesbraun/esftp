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
