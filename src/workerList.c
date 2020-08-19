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
 * Worker list
 * This modules implements a worker list to keep track of the active workers.
 * Important: this module is not thread safe.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "worker.h"
#include "workerList.h"

/**
 * Initializes the worker list
 */
int wlInitialize(struct WorkerList* workerList)
{
        workerList->tidArray = (struct WorkerConfig**) malloc(INITIAL_SIZE * sizeof(struct WorkerConfig*));
        if (workerList->tidArray == NULL) {
                fprintf(stderr, "An error ocurred while allocating memory for the worker list.\n");
                return -1;
        }
        workerList->arraySize = 1u;
        workerList->usedSlots = 0u;

        return 0;
}

/**
 * Cleans up the worker list by removing finished workers
  * The internal array will get resized afterwards if necessary
 */
int wlCleanup(struct WorkerList* workerList)
{
        unsigned int i;
        int tmp;

        for (i = 0; i < workerList->usedSlots; i++) {
                if (workerList->tidArray[i]->finished == 1) {
                        // Delete from list
                        tmp = wlDelete(workerList, i);
                        if (tmp == -1) {
                                // Deleting not successful
                                return -1;
                        }
                }
        }

        if ((((float) workerList->usedSlots) / ((float) workerList->arraySize)) < 0.5f) {
                // Resize array
                tmp = wlResize(workerList, workerList->usedSlots * RESIZE_FACTOR + 1);
                if (tmp == -1) {
                        // Resizing not successful (should not occur since this is only downsizing)
                        return -1;
                }
        }

        return 0;
}

/**
 * Deletes the worker at the given index from the list
 */
int wlDelete(struct WorkerList* workerList, unsigned int index)
{
        // Using long long here to be able to store the highest possible index plus 1
        unsigned long long int i;

        if (index >= workerList->usedSlots) {
                // Index invalid
                fprintf(stderr, "Deleting worker from list not successful: index out of bounds\n");
                return -1;
        }

        // Free space of worker config
        free(workerList->tidArray[index]);

        // Move elements
        for (i = index + 1; i < workerList->usedSlots; i++) {
                workerList->tidArray[i - 1] = workerList->tidArray[i];
        }

        // Last slot is not used anymore
        workerList->usedSlots--;

        return 0;
}

/**
 * Adds a worker to the list
 */
int wlAdd(struct WorkerList* workerList, struct WorkerConfig* workerConfig)
{
        int tmp;

        if (workerList->arraySize <= workerList->usedSlots) {
                // Resize array
                tmp = wlResize(workerList, workerList->usedSlots * RESIZE_FACTOR + 1);
                if (tmp == -1) {
                        // Resizing not successful
                        return -1;
                }
        }

        workerList->tidArray[workerList->usedSlots] = workerConfig;
        workerList->usedSlots++;

        return 0;
}

/**
 * Tries to resize the internal array to the given size
 */
int wlResize(struct WorkerList* workerList, unsigned int newArraySize)
{
        if (newArraySize < workerList->usedSlots) {
                // Cannot resize (too small)
                fprintf(stderr, "Resizing worker list failed: too small\n");
                return -1;
        }

        // Backup old pointer
        struct WorkerConfig** tidOldArray = workerList->tidArray;

        workerList->tidArray = (struct WorkerConfig**) realloc(tidOldArray, newArraySize * sizeof(struct WorkerConfig*));
        if (workerList->tidArray == NULL) {
                // Allocating memory not successful
                fprintf(stderr, "Resizing worker list failed: realloc failed\n");
                workerList->tidArray = tidOldArray;
                return -1;
        }

        workerList->arraySize = newArraySize;

        return 0;
}

/**
 * Waits for all workers to finish.
 * This function is blocking.
 */
int wlJoin(struct WorkerList* workerList)
{
        int tmp;

        while (workerList->usedSlots != 0) {
                tmp = pthread_join(workerList->tidArray[0]->tid, NULL);
                if (tmp == 0) {
                        tmp = wlDelete(workerList, 0);
                        if (tmp == -1) {
                                return -1;
                        }
                } else {
                        return -1;
                }
        }

        tmp = wlCleanup(workerList);
        if (tmp == -1) {
                return -1;
        }

        return 0;
}

/**
 * Frees the space of the internal array
 */
void wlFree(struct WorkerList* workerList)
{
        free(workerList->tidArray);
}
