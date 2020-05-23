//poll version of select.c
//poll is improved select
#include "csapp.h"

void command()
{
    char buf[MAXLINE];
    if (Fgets(buf, MAXLINE, stdin) == 0)
    {
        exit(0);
    }
    puts(buf);
}

void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    if ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) //3. read it from client
    {
        printf("server received: %d bytes\n", (int)n);
        Rio_writen(connfd, buf, n); //4. write it back to client
    }
}

int main(int argc, char const *argv[])
{
    int listenfd;
    int connfd;
    struct sockaddr_storage clientaddr;
    socklen_t clientlen = sizeof(struct sockaddr_storage);
    nfds_t nfds = 2;
    struct pollfd *pfd;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = open_listenfd(argv[1]);
    pfd = calloc(2, sizeof(struct pollfd));

    pfd[0].fd = STDIN_FILENO;
    pfd[0].events = POLLIN;
    pfd[1].fd = listenfd;
    pfd[1].events = POLLIN;

    while (1)
    {
        Poll(pfd, 2, -1);
        if (pfd[0].revents & POLLIN)
        {
            command();
        }
        if (pfd[1].revents & POLLIN)
        {
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            echo(connfd);
            Close(connfd); // note we close the connection here
        }
    }
    return 0;
}
