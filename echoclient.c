#include "csapp.h"

int main(int argc, char const *argv[])
{
    int clientfd;
    char *host;
    char *port;
    char buf[MAXLINE];
    rio_t rio;
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = argv[2];
    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);
    while (Fgets(buf, MAXLINE, stdin) != NULL) //1. read from stdin
    {
        Rio_writen(clientfd, buf, strlen(buf)); //2. write it to server
        Rio_readlineb(&rio, buf, sizeof(buf));  //5. read from server
        Fputs(buf, stdout);                     // 6.print it
    }
    close(clientfd);
    exit(0);
}
