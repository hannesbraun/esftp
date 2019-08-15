#include <limits.h>
#include <pthread.h>

#include "server.h"

#define RESIZE_FACTOR 1.42

typedef struct WorkerList_t
{
    WorkerArguments** tidArray;
    unsigned int uiArraySize;
    unsigned int uiUsedSlots;
} WorkerList;

int wlInitialize(WorkerList* pWorkerList);
int wlCleanup(WorkerList* pWorkerList);
int wlDelete(WorkerList* pWorkerList, int iIndex);
int wlAdd(WorkerList* pWorkerList, WorkerArguments* workerArguments);
int wlResize(WorkerList* pWorkerList, unsigned int uiNewArraySize);
int wlJoin(WorkerList* pWorkerList);
void wlFree(WorkerList* pWorkerList);
