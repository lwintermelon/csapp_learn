//simpe select usage example
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
    fd_set read_set;
    fd_set ready_set;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = open_listenfd(argv[1]);
    FD_ZERO(&read_set);
    FD_SET(STDIN_FILENO, &read_set);
    FD_SET(listenfd, &read_set);
    while (1)
    {
        ready_set = read_set;
        Select(listenfd + 1, &ready_set, NULL, NULL, NULL);
        if (FD_ISSET(STDIN_FILENO, &ready_set))
        {
            command();
        }
        if (FD_ISSET(listenfd, &ready_set))
        {
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            echo(connfd);
            Close(connfd); // note we close the connection here
        }
    }
    return 0;
}
