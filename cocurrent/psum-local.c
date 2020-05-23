//caculate sum(1..=2^n) by multi-threading
#include "csapp.h"
#define MAXTHREADS 4

long global_sum = 0;
long nelems_per_thread;
long psum[MAXTHREADS];
sem_t mutex;

void *sum(void *ptoid)
{
    long id = *((long *)ptoid);
    long sum = 0;
    long start = id * nelems_per_thread + 1;
    long end = start + nelems_per_thread - 1;
    for (long l = start; l <= end; l++)
    {
        sum += l;
    }
    psum[id] = sum;
    return NULL;
}

int main(int argc, char const *argv[])
{
    long nelems;
    long nthreads;
    long myid[MAXTHREADS];
    pthread_t pids[MAXTHREADS];
    long n;

    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <num of threads> <n of 2^n>\n", argv[0]);
        exit(0);
    }
    nthreads = atoi(argv[1]);
    n = atoi(argv[2]);
    nelems = 1L << n;
    nelems_per_thread = nelems / nthreads;
    for (int i = 0; i < nthreads; i++)
    {
        myid[i] = i;
        Pthread_create(&pids[i], NULL, sum, &myid[i]);
    }
    for (int i = 0; i < nthreads; i++)
    {
        pthread_join(pids[i], NULL);
    }
    for (int i = 0; i < nthreads; i++)
    {
        global_sum += psum[i];
    }

    if (global_sum == ((1 + nelems) * nelems) / 2)
    {
        printf("sum(1 ..= 2 ^ n) is %ld\n", global_sum);
    }
    else
    {
        printf("Error: wrong result=%ld\n", global_sum);
    }

    return 0;
}
