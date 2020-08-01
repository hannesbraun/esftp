#include <limits.h>
#include <pthread.h>

#include "server.h"
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
