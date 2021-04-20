#include <stdio.h>
#include <pthread.h>


#define THREAD_CNT 5


static void
worker_func(void) {
    printf("Hello from worker %d\n", pthread_self());
    return;
}


int
main(void) {
    pthread_t workers[THREAD_CNT];
    for (int i = 0; i < THREAD_CNT; ++i) {
        pthread_create(&workers[i], NULL, worker_func, NULL);
    }

    for (int i = 0; i < THREAD_CNT; ++i) {
        pthread_join(workers[i], NULL);
    }

    printf("Hello from main thread\n");
}
