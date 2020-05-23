#include "csapp.h"

int main(int argc, char const *argv[])
{
    struct addrinfo *p;
    struct addrinfo *plist;
    struct addrinfo hints;
    char buf[MAXLINE];
    int rc;
    int flags;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <domain name>\n", argv[0]);
        exit(0);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((rc = getaddrinfo(argv[1], NULL, &hints, &plist)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
        exit(1);
    }

    flags = NI_NUMERICHOST;
    for (p = plist; p != NULL; p = p->ai_next)
    {
        Getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAXLINE, NULL, 0, flags);
        printf("%s\n", buf);
    }

    Freeaddrinfo(plist);

    exit(0);
}
