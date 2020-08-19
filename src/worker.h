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
