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

#ifndef workerList_h
#define workerList_h

#include "worker.h"

#define INITIAL_SIZE 1
#define RESIZE_FACTOR 1.42

struct WorkerList {
        struct WorkerConfig** tidArray;
        unsigned int arraySize;
        unsigned int usedSlots;
} WorkerList;

int wlInitialize(struct WorkerList* workerList);
int wlCleanup(struct WorkerList* workerList);
int wlDelete(struct WorkerList* workerList, unsigned int index);
int wlAdd(struct WorkerList* workerList, struct WorkerConfig* workerConfig);
int wlResize(struct WorkerList* workerList, unsigned int newArraySize);
int wlJoin(struct WorkerList* workerList);
void wlFree(struct WorkerList* workerList);

#endif
