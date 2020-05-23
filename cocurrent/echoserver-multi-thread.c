#include "csapp.h"

void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) //3. read it from client
    {
        printf("server received: %d bytes\n", (int)n);
        Rio_writen(connfd, buf, n); //4. write it back to client
    }
}

void *thread_routine(void *parg)
{

    int connfd = (int)parg;
    echo(connfd);
}

int main(int argc, char const *argv[])
{
    int listenfd;
    int connfd;
    pthread_t pid;
    socklen_t clientlen = sizeof(struct sockaddr_storage);
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE];
    char client_port[MAXLINE];

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
    }
    listenfd = open_listenfd(argv[1]);
    while (1)
    {
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);

        //note we can't pass &connfd to get connfd in new thread, it will cause data race.
        //we just pass connfd then cast void* to int...
        pthread_create(&pid, NULL, thread_routine, (void *)connfd);
    }
    return 0;
}
