#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

const int NUMTHREADS = 3;
const int SIZE = 10;
const int chunk_size = SIZE / NUMTHREADS;
int primes[10] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29 };

void* routine(void* arg) {
    int index = *(int*)arg;
    int sum = 0;
    for (int j = 0; j < SIZE/NUMTHREADS; j++) {
        sum += primes[index + j];
    }
    printf("Local sum: %d\n", sum);
    *(int*)arg = sum;
    return arg;
}

void* multiThread_Handler(void *arg)
{
    int thread_index = *((int *)arg);

    unsigned int start_index = (thread_index * chunk_size);
    unsigned int end_index = start_index + chunk_size;
    if (thread_index == NUMTHREADS - 1){
        end_index = SIZE;
    }

    // printf("Index: %d\n", thread_index);
    // printf("Start Index: %d\n", start_index);
    // printf("End Index: %d\n", end_index);

    int local_sum = 0;
    for(int i =  start_index; i < end_index;  i++)
    {
        printf("Processing array element at: %d\n", i);
        local_sum += primes[i];
    }

    *((int*)arg) = local_sum;
    return arg;
}

int main(int argc, char* argv[]) {
    // printf("SIZE: %d\n", SIZE);
    // printf("NUMTHREADS: %d\n", NUMTHREADS);
    pthread_t th[NUMTHREADS];
    int i;
    for (i = 0; i < NUMTHREADS; i++) {
        int* a = malloc(sizeof(int));
        *a = i ; //* SIZE/NUMTHREADS;
        if (pthread_create(&th[i], NULL, &multiThread_Handler, a) != 0) {
            perror("Failed to create thread");
        }
    }

    size_t globalSum = 0;
    for (i = 0; i < NUMTHREADS; i++) {
        int* r;
        if (pthread_join(th[i], (void**) &r) != 0) {
            perror("Failed to join thread");
        }
        globalSum += *r;
        free(r);
    }
    printf("Global sum: %ld\n", globalSum);
    return 0;
}