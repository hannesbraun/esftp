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

/*
 * Module for printing the status while receiving a file
 */

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "recvFileStatus.h"

enum ByteUnit {
        Byte,
        Kibibyte,
        Mebibyte,
        Gibibyte,
        Tebibyte,
        Pebibyte,
        Exbibyte
};

enum ByteUnit getUnit(uint64_t size);
uint64_t convertBytes(uint64_t bytes, enum ByteUnit unit, unsigned int* dec);
void calcDec(unsigned int* dec, uint64_t in, unsigned int i);
void getUnitStr(enum ByteUnit unit, char* str);

void printStatus(uint64_t* byteProgress, struct timeval* timeProgress, uint64_t size, unsigned char erase)
{
        unsigned int i;

        // Units to use
        enum ByteUnit unitSize;
        enum ByteUnit unitSpeed;

        // Bytes transferred between the two timestamps
        uint64_t transferred;

        // Time elapsed between the two timestamps
        uint64_t timeDiff;

        // Speed in bytes per second
        uint64_t speed;

        // Output: how many (kibi/mebi/...)bytes were received
        uint64_t outRecv;

        // Output: total size of the file in (kibi/mebi/...)bytes
        uint64_t outTotal;

        // Decimals for output
        unsigned int decRecv;
        unsigned int decTotal;
        unsigned int decSpeed;

        // ETA in mins and secs
        unsigned int min;
        unsigned int sec;

        // Bytes left to receive
        uint64_t bytesLeft = size - byteProgress[20];

        // Output string
        char out[1024];

        // Unit strings
        char unitSizeStr[4];
        char unitSpeedStr[4];

        // Move cursor backwards
        printf("\x1b[1024D");

        if (erase == 1) {
                // Erase status and exit function
                for (i = 0; i < strlen(out); i++) {
                        putchar(' ');
                }
                printf("\x1b[1024D");

                return;
        }
        // Else: print/update status

        // Get file size unit
        unitSize = getUnit(size);
        getUnitStr(unitSize, unitSizeStr);

        // Calculate bytes transfered and time difference for speed calculation
        transferred = byteProgress[20] - byteProgress[0];
        timeDiff = (timeProgress[20].tv_sec * 1000000 + timeProgress[20].tv_usec) - (timeProgress[0].tv_sec * 1000000 + timeProgress[0].tv_usec);

        // Calculate speed in bytes per second
        speed = (transferred * 1000000) / timeDiff;

        // Calculate ETA
        if (speed > 0) {
                min = (bytesLeft / speed) / 60;
                sec = (bytesLeft / speed) % 60;
        } else {
                // Avoid division by zero
                min = UINT_MAX;
                sec = UINT_MAX;
        }

        // Get speed unit
        unitSpeed = getUnit(transferred);
        transferred = convertBytes(transferred, unitSpeed, &decSpeed);
        getUnitStr(unitSpeed, unitSpeedStr);

        // Convert received and total bytes
        outRecv = convertBytes(byteProgress[20], unitSize, &decRecv);
        outTotal = convertBytes(size, unitSize, &decTotal);

        // Write new status
        snprintf(out, 1024, "%" PRIu64 ".%.2u %s / %" PRIu64 ".%.2u %s | %" PRIu64 ".%.2u %s/s | ETA: %dm %ds     ",
                 outRecv,
                 decRecv,
                 unitSizeStr,
                 outTotal,
                 decTotal,
                 unitSizeStr,
                 (transferred * 1000000) / timeDiff,
                 (unsigned int)((decSpeed * 1000000) / timeDiff),
                 unitSpeedStr,
                 min,
                 sec);

        // Print generated status string
        printf("%s", out);

        fflush(stdout);
}

/**
 * Returns an appropriate unit for the given amount of bytes.
 * Target: the printed size should not be longer than three characters.
 */
enum ByteUnit getUnit(uint64_t size)
{
        if (size < 1000) {
                return Byte;
        } else if (size < 1000000) {
                return Kibibyte;
        } else if (size < 1000000000) {
                return Mebibyte;
        } else if (size < 1000000000000) {
                return Gibibyte;
        } else if (size < 1000000000000000) {
                return Tebibyte;
        } else if (size < 1000000000000000000) {
                return Pebibyte;
        } else {
                return Exbibyte;
        }
}

/**
 * Converts the given amount of bytes into another byte unit.
 * dec will contain the first two decimals of the result
 */
uint64_t convertBytes(uint64_t bytes, enum ByteUnit unit, unsigned int* dec)
{
        uint64_t retVal = 0;
        uint64_t tmpDec;

        switch (unit) {
                case Byte:
                        retVal = bytes;
                        (*dec) = 0;
                        break;
                case Kibibyte:
                        retVal = bytes >> 10;
                        tmpDec = bytes - (retVal << 10);
                        calcDec(dec, tmpDec, 1);
                        break;
                case Mebibyte:
                        retVal = bytes >> 20;
                        tmpDec = bytes - (retVal << 20);
                        calcDec(dec, tmpDec, 2);
                        break;
                case Gibibyte:
                        retVal = bytes >> 30;
                        tmpDec = bytes - (retVal << 30);
                        calcDec(dec, tmpDec, 3);
                        break;
                case Tebibyte:
                        retVal = bytes >> 40;
                        tmpDec = bytes - (retVal << 40);
                        calcDec(dec, tmpDec, 4);
                        break;
                case Pebibyte:
                        retVal = bytes >> 50;
                        tmpDec = bytes - (retVal << 50);
                        calcDec(dec, tmpDec, 5);
                        break;
                case Exbibyte:
                        retVal = bytes >> 60;
                        tmpDec = bytes - (retVal << 60);
                        calcDec(dec, tmpDec, 6);
                        break;
        }

        return retVal;
}

/**
 * Calculates the first two decimals
 */
void calcDec(unsigned int* dec, uint64_t in, unsigned int i)
{
        double tmp = in;
        tmp = tmp / pow(2, i * 10);
        tmp = tmp * 100;
        (*dec) = (unsigned int) tmp;
}

/**
 * Copies the appropriate unit string into the provided buffer
 */
void getUnitStr(enum ByteUnit unit, char* str)
{
        switch (unit) {
                case Byte:
                        strncpy(str, "B", 2);
                        break;
                case Kibibyte:
                        strncpy(str, "KiB", 4);
                        break;
                case Mebibyte:
                        strncpy(str, "MiB", 4);
                        break;
                case Gibibyte:
                        strncpy(str, "GiB", 4);
                        break;
                case Tebibyte:
                        strncpy(str, "TiB", 4);
                        break;
                case Pebibyte:
                        strncpy(str, "PiB", 4);
                        break;
                case Exbibyte:
                        strncpy(str, "EiB", 4);
                        break;
        }
}
