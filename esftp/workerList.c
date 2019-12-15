/**
 * @file workerList.c
 * @brief File contains the worker list's functions.
 * @author Hannes Braun
 * @date 10.08.2019
 *
 * This module is not thread safe.
 */

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "server.h"
#include "worker.h"
#include "workerList.h"

int wlInitialize(struct WorkerList* workerList)
{
        workerList->tidArray = (struct WorkerConfig**) malloc(1 * sizeof(struct WorkerConfig*));
        if (workerList->tidArray == NULL) {
                fprintf(stderr, "An error ocurred while allocating memory for the worker list.\n");
                return -1;
        }
        workerList->arraySize = 1u;
        workerList->usedSlots = 0u;

        return 0;
}

int wlCleanup(struct WorkerList* workerList)
{
        int i;
        int retVal;

        for (i = 0; i < workerList->usedSlots; i++) {
                if (workerList->tidArray[i]->finished == 1) {
                        // Delete from list
                        retVal = wlDelete(workerList, i);
                        if (retVal == -1) {
                                // Deleting not successful
                                return -1;
                        }
                }
        }

        if ((((float) workerList->usedSlots) / ((float) workerList->arraySize)) < 0.5f) {
                // Resize array
                retVal = wlResize(workerList, workerList->usedSlots * RESIZE_FACTOR + 1);
                if (retVal == -1) {
                        // Resizing not successful (should not occur since this is only downsizing)
                        return -1;
                }
        }

        return 0;
}

int wlDelete(struct WorkerList* workerList, int index)
{
        int i;

        if (index >= workerList->usedSlots || index < 0) {
                // Index invalid
                fprintf(stderr, "Deleting worker from list not successful: index out of bounds\n");
                return -1;
        }

        // Free space of worker arguments
        free(workerList->tidArray[index]);

        // Move elements
        for (i = index + 1; i < workerList->usedSlots; i++) {
                workerList->tidArray[i - 1] = workerList->tidArray[i];
        }

        // Last slot is not used anymore
        workerList->usedSlots--;

        return 0;
}

int wlAdd(struct WorkerList* workerList, struct WorkerConfig* workerConfig)
{
        int retVal;

        if (workerList->arraySize <= workerList->usedSlots) {
                // Resize array
                retVal = wlResize(workerList, workerList->usedSlots * RESIZE_FACTOR + 1);
                if (retVal == -1) {
                        // Resizing not successful
                        return -1;
                }
        }

        workerList->tidArray[workerList->usedSlots] = workerConfig;
        workerList->usedSlots++;

        return 0;
}

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

int wlJoin(struct WorkerList* workerList)
{
        int retVal;

        while (workerList->usedSlots != 0) {
                retVal = pthread_join(workerList->tidArray[0]->tid, NULL);
                if (retVal == 0) {
                        retVal = wlDelete(workerList, 0);
                        if (retVal == -1) {
                                return -1;
                        }
                } else {
                        return -1;
                }
        }

        retVal = wlCleanup(workerList);
        if (retVal == -1) {
                return -1;
        }

        return 0;
}

void wlFree(struct WorkerList* workerList)
{
        free(workerList->tidArray);
}
