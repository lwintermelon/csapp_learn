// multi-process echo server
#include "csapp.h"

void sigchld_handler(int sig)
{
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        sio_putl(pid);
        sio_puts(" exit\n");
    }
    return;
}
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

int main(int argc, char const *argv[])
{
    int listenfd;
    int connfd;
    socklen_t clientlen = sizeof(struct sockaddr_storage);
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE];
    char client_port[MAXLINE];

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
    }

    Signal(SIGCHLD, sigchld_handler);

    listenfd = open_listenfd(argv[1]);
    while (1)
    {
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        if (Fork() == 0)
        {
            Close(listenfd);
            Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
            printf("Connected to (%s, %s)\n", client_hostname, client_port);
            echo(connfd);
            Close(connfd);
            exit(0);
        }
        Close(connfd);
    }
    return 0;
}
