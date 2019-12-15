/**
 * Worker header
 */

#ifndef worker_h
#define worker_h

#include <stdint.h>
#include "pthread.h"

struct WorkerConfig {
        int socketID;
        char** items;
        unsigned long int itemsLen;
        pthread_t tid;
        unsigned char finished: 1;
};

void* worker(void* vConfig);

#endif
