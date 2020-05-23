#include "csapp.h"

#define NUM_THREADS 4
#define SBUF_SIZE 16
// a double-end array queue
// interestingly, using PV we don't have to manage the state of
// empty or full, it will suspend and wait for insert or remove
// but we can't insert and remove on the same thread, it may cause dead-lock.
typedef struct
{
    int *buf;    //array buf
    int n;       // max number of queue
    int front;   //if queue isn't empty, buf[(front+1) % n] is first item
    int rear;    //if queue isn't empty, buf[rear % n] is last item
    sem_t mutex; // mutex for buf
    sem_t slots; //number of avilable slots
    sem_t items; //number of avilable items
} sbuf_t;

void sbuf_init(sbuf_t *psbuf, int n)
{
    psbuf->n = n;
    psbuf->buf = malloc(n * sizeof(int));
    psbuf->front = 0;
    psbuf->rear = 0;
    Sem_init(&psbuf->mutex, 0, 1);
    Sem_init(&psbuf->slots, 0, n);
    Sem_init(&psbuf->items, 0, 0);
}

void sbuf_deinit(sbuf_t *psbuf)
{
    Free(psbuf->buf);
}

void sbuf_insert(sbuf_t *psbuf, int value)
{
    P(&psbuf->slots);

    P(&psbuf->mutex);
    psbuf->rear = (psbuf->rear + 1) % psbuf->n;
    (psbuf->buf)[psbuf->rear] = value;
    V(&psbuf->mutex);

    V(&psbuf->items);
}

int sbuf_remove(sbuf_t *psbuf)
{
    P(&psbuf->items);

    P(&psbuf->mutex);
    int value = (psbuf->buf)[(psbuf->front + 1) % psbuf->n];
    psbuf->front += 1;
    V(&psbuf->mutex);

    V(&psbuf->slots);
    return value;
}

sbuf_t sbuf;
static int byte_count = 0;
static sem_t mutex;

void echo_count(int connfd)
{
    int n;
    char buf[MAXLINE];
    rio_t rio;
    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
    {
        P(&mutex);
        byte_count += n;
        printf("server received: %d (%d total) bytes on fd %d\n", (int)n, byte_count, connfd);
        V(&mutex);
        Rio_writen(connfd, buf, n);
    }
}

void *thread_routine(void *parg)
{
    Pthread_detach(pthread_self());
    while (1)
    {
        int connfd = sbuf_remove(&sbuf);
        echo_count(connfd);
        Close(connfd);
    }
}
sbuf_t sbuf;

int main(int argc, char const *argv[])
{
    sem_init(&mutex, 0, 1);
    int listenfd;
    int connfd;
    pthread_t pids[NUM_THREADS];
    socklen_t clientlen = sizeof(struct sockaddr_storage);
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE];
    char client_port[MAXLINE];

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = open_listenfd(argv[1]);
    sbuf_init(&sbuf, SBUF_SIZE);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        Pthread_create(&pids[i], NULL, thread_routine, NULL);
    }

    while (1)
    {
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);

        //note we can't pass &connfd to get connfd in new thread, it will cause data race.
        //we just pass connfd then cast void* to int...
        sbuf_insert(&sbuf, connfd);
    }
    return 0;
}
