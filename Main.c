#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

#define totalThreads 4
#define chunkSize (1000000 / totalThreads)

bool isPrime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6)
        if (n % i == 0 || n % (i + 2) == 0) return false;
    return true;
}

typedef struct {
    int start;
} unitThread;

void* printPrimes(void* param) {
    unitThread *data = (unitThread*)param;
    for (int i = 0; i < chunkSize; i++)
        if (isPrime(data->start + i))
            printf("%d\n", data->start + i);
    return NULL;
}

long getTimeInMilliseconds() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return (time.tv_sec * 1000) + (time.tv_usec / 1000);
}

int main() {

    pthread_t threadHandle[totalThreads];
    unitThread thread[totalThreads];
    int startPoint[totalThreads] = {0, 250000, 500000, 750000};

    long startTime = getTimeInMilliseconds();
    
    for (int i = 0; i < totalThreads; i++) {
        thread[i].start = startPoint[i];
        if (pthread_create(&threadHandle[i], NULL, printPrimes, &thread[i]) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            return 1;
        }
    }

    for (int i = 0; i < totalThreads; i++)
        pthread_join(threadHandle[i], NULL);

    long endTime = getTimeInMilliseconds();
    printf("ETT: %lu milliseconds\n", endTime - startTime);
    return 0;
}