/**
 * Worker header
 */

struct WorkerConfig {
        int socketID;
        char** items;
        pthread_t tid;
        unsigned char finished: 1;
}

void* worker(void* vConfig);
