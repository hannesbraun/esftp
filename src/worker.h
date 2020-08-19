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
 * Worker header
 */

#ifndef worker_h
#define worker_h

#include <pthread.h>
#include <stdint.h>

struct WorkerConfig {
        int socketID;
        char** items;
        unsigned long int itemsLen;
        pthread_t tid;
        unsigned char finished: 1;
        unsigned char selfFree: 1;
};

void* worker(void* vConfig);

#endif
