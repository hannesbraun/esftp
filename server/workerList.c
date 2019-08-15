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
#include "workerList.h"

int wlInitialize(WorkerList* pWorkerList)
{
    pWorkerList->tidArray = (WorkerArguments**) malloc(1 * sizeof(WorkerArguments*));
    if (pWorkerList->tidArray == NULL)
    {
        return -1;
    }
    pWorkerList->uiArraySize = 1u;
    pWorkerList->uiUsedSlots = 0u;

    return 0;
}

int wlCleanup(WorkerList* pWorkerList)
{
    int iCounter;
    int iReturnValue;

    for (iCounter = 0; iCounter < pWorkerList->uiUsedSlots; iCounter++)
    {
        if (pWorkerList->tidArray[iCounter]->ucFinished == 1)
        {
            // Delete from list
            iReturnValue = wlDelete(pWorkerList, iCounter);
            if (iReturnValue == -1)
            {
                // Deleting not successful
                return -1;
            }
        }
    }

    if ((((float) pWorkerList->uiUsedSlots) / ((float) pWorkerList->uiArraySize)) < 0.5f)
    {
        // Resize array
        iReturnValue = wlResize(pWorkerList, pWorkerList->uiUsedSlots * RESIZE_FACTOR + 1);
        if (iReturnValue == -1)
        {
            // Resizing not successful (should not occur since this is only downsizing)
            return -1;
        }
    }

    return 0;
}

int wlDelete(WorkerList* pWorkerList, int iIndex)
{
    int iCounter;

    if (iIndex >= pWorkerList->uiUsedSlots || iIndex < 0)
    {
        // Index invalid
        return -1;
    }

    // Free space of worker arguments
    free(pWorkerList->tidArray[iIndex]);

    // Move elements
    for (iCounter = iIndex + 1; iCounter < pWorkerList->uiUsedSlots; iCounter++)
    {
        pWorkerList->tidArray[iCounter - 1] = pWorkerList->tidArray[iCounter];
    }

    // Last slot is not used anymore
    pWorkerList->uiUsedSlots--;

    return 0;
}

int wlAdd(WorkerList* pWorkerList, WorkerArguments* workerArguments)
{
    int iReturnValue;

    if (pWorkerList->uiArraySize == pWorkerList->uiUsedSlots)
    {
        // Resize array
        iReturnValue = wlResize(pWorkerList, pWorkerList->uiUsedSlots * RESIZE_FACTOR + 1);
        if (iReturnValue == -1)
        {
            // Resizing not successful
            return -1;
        }
    }

    pWorkerList->tidArray[pWorkerList->uiUsedSlots] = workerArguments;
    pWorkerList->uiUsedSlots++;

    return 0;
}

int wlResize(WorkerList* pWorkerList, unsigned int uiNewArraySize)
{
    if (uiNewArraySize < pWorkerList->uiUsedSlots)
    {
        // Cannot resize (too small)
        return -1;
    }

    WorkerArguments** tidOldArray = pWorkerList->tidArray;

    pWorkerList->tidArray = (WorkerArguments**) realloc(tidOldArray, uiNewArraySize * sizeof(WorkerArguments*));
    if (pWorkerList->tidArray == NULL)
    {
        // Allocating memory not successful
        pWorkerList->tidArray = tidOldArray;
        return -1;
    }

    pWorkerList->uiArraySize = uiNewArraySize;

    return 0;
}

int wlJoin(WorkerList* pWorkerList)
{
    int iReturnValue;

    while (pWorkerList->uiUsedSlots != 0)
    {
        iReturnValue = pthread_join(pWorkerList->tidArray[0]->tid, NULL);
        if (iReturnValue == 0)
        {
            iReturnValue = wlDelete(pWorkerList, 0);
            if (iReturnValue == -1)
            {
                fprintf(stderr, "An error ocurred while deleting the worker from the worker list.\n");
            }
        }
        else
        {
            return -1;
        }
    }

    wlCleanup(pWorkerList);

    return 0;
}

void wlFree(WorkerList* pWorkerList)
{
    free(pWorkerList->tidArray);
}
