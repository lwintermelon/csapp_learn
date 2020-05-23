// kqueue version of poll.c
// kqueue offers features beyond poll
#include "csapp.h"
#include <sys/event.h>

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

    struct kevent event[2];  //Event we want to monitor
    struct kevent tevent[2]; // Event triggered

    int kq;
    int fd;
    int ret;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = open_listenfd(argv[1]);

    if ((kq = kqueue()) == -1)
    {
        unix_error("kqueue() failed");
    }

    EV_SET(&event[0], STDIN_FILENO, EVFILT_READ, EV_ADD | EV_ENABLE, 0,
           0, NULL);

    EV_SET(&event[1], listenfd, EVFILT_READ, EV_ADD | EV_ENABLE, 0,
           0, NULL);

    if (kevent(kq, event, 2, NULL, 0, NULL) == -1)
    {
        unix_error("kevent register error");
    }

    while (1)
    {

        if ((ret = kevent(kq, NULL, 0, tevent, 2, NULL)) == -1)
        {
            unix_error("kevent wait error");
        }
        for (int i = 0; i < ret; i++)
        {
            if (tevent[i].ident == STDIN_FILENO)
            {
                command();
            }
            if (tevent[i].ident == listenfd)
            {
                connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
                echo(connfd);
                Close(connfd); // note we close the connection here
            }
        }
    }
    return 0;
}
