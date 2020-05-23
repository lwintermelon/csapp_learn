#include "csapp.h"
#define MAX_CONNECTIONS 1024

typedef struct
{
    struct pollfd *pfds;
    int nready;
    int nfds;
    struct pollfd *listen_poolfd;
    int clientfd[MAX_CONNECTIONS];
    rio_t clientrio[MAX_CONNECTIONS];
} Pool;

void init_pool(int listenfd, Pool *p);
void add_client(int connfd, Pool *p);
void check_clients(Pool *p);
int byte_count = 0;

int main(int argc, char const *argv[])
{
    int listenfd;
    int connfd;
    socklen_t clientlen = sizeof(struct sockaddr_storage);
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE];
    char client_port[MAXLINE];

    static Pool pool;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
    }

    listenfd = Open_listenfd(argv[1]);
    init_pool(listenfd, &pool);
    int n = 0;
    while (1)
    {
        pool.nready = Poll(pool.pfds, MAX_CONNECTIONS, -1);

        if (pool.listen_poolfd->revents & POLLIN) //if a connection comes in
        {
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
            printf("Connected to (%s, %s)\n", client_hostname, client_port);
            add_client(connfd, &pool);
            pool.nready -= 1; // we have processed an event, pool.listen_poolfd
        }
        //check clients event
        check_clients(&pool);
    }
    return 0;
}

void init_pool(int listenfd, Pool *p)
{
    p->nfds = 1;
    p->pfds = calloc(MAX_CONNECTIONS, sizeof(struct pollfd));
    (p->pfds)[0].fd = listenfd;
    (p->pfds)[0].events = POLLIN;
    p->listen_poolfd = &((p->pfds)[0]);

    for (int i = 1; i < 1024; i++) //set unused pollfd.fd to -1
    {
        (p->pfds)[i].fd = -1;
    }
}

void add_client(int connfd, Pool *p)
{
    int i;
    for (i = 1; i < MAX_CONNECTIONS; i++)
    {
        if ((p->pfds)[i].fd < 0)
        {
            (p->pfds)[i].fd = connfd;
            (p->pfds)[i].events = POLLIN; // don't forget to add the event
            Rio_readinitb(&(p->clientrio)[i], connfd);
            return;
        }
    }
    //can't find a place
    app_error("add_client error: Too many clents");
}

void check_clients(Pool *p)
{
    int connfd;
    int n;
    char buf[MAXLINE];
    rio_t rio;
    for (int i = 1; (i < MAX_CONNECTIONS) && (p->nready > 0); i++)
    {
        struct pollfd connpollfd = (p->pfds)[i];
        connfd = connpollfd.fd;
        if ((connfd > 0) && (connpollfd.revents & POLLIN)) // the descriptor is ready
        {
            if ((n = Rio_readlineb(&(p->clientrio)[i], buf, MAXLINE)) != 0)
            {
                byte_count += n;
                printf("Srever received %d (%d total) bytes on fd %d\n", n, byte_count, connfd);
                Rio_writen(connfd, buf, MAXLINE);
            }
            //EOF detected, close it and set it to -1
            else
            {
                Close(connfd);
                (p->pfds)[i].fd = -1;
            }
        }
    }
}