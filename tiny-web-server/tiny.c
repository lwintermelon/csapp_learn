#include "csapp.h"

void doit(int fd);
void read_request_headers(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);
int main(int argc, char const *argv[])
{
    int listenfd;
    int connfd;
    char hostname[MAXLINE];
    char port[MAXLINE];
    socklen_t clientlen = sizeof(struct sockaddr_storage);
    struct sockaddr_storage clientaddr;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1)
    {
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);
        Close(connfd);
    }

    return 0;
}

void doit(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE];
    char method[MAXLINE];
    char uri[MAXLINE];
    char version[MAXLINE];

    char filename[MAXLINE];
    char cgiargs[MAXLINE];

    rio_t rio;

    Rio_readinitb(&rio, fd);
    if (Rio_readlineb(&rio, buf, MAXLINE) == 0) //read an EOF
    {
        return;
    }
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);

    if (strcasecmp(method, "GET") != 0) // not a GET method
    {
        clienterror(fd, method, "501", "Not Implemented", "tiny doesn't implement this method");
        return;
    }
    read_request_headers(&rio); // ignore all headers

    is_static = parse_uri(uri, filename, cgiargs);
    if (stat(filename, &sbuf) != 0) // stat failed
    {
        clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
        return;
    }

    if (is_static) //serve static content
    {
        if (!(S_ISREG(sbuf.st_mode) || !(S_IRUSR & sbuf.st_mode))) // not a regular file OR doen't have read permission
        {
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);
    }
    else //serve dynamic content
    {
        if (!(S_ISREG(sbuf.st_mode) || !(S_IXUSR & sbuf.st_mode))) // not a regular file OR doen't have execute permission
        {
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);
    }
}

void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
    char buf[MAXLINE];

    //print the http response headers
    // we doesn't generate Content-length, is it ok?
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    //print the http response body
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body style=\"background-color:white\">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s</p>\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "</hr><em>The Tiny Web Server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "</body></html>");
    Rio_writen(fd, buf, strlen(buf));
}

void read_request_headers(rio_t *rp)
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    while (strcmp(buf, "\r\n") != 0) // read until \r\n, the end of request headers
    {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

/*
 * parse_uri- parse URI into filename and CGI args
 *            return 1 if static, 0 if dynamic
*/
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;
    if (!strstr(uri, "cgi-bin")) //uri doesn't contain "cgi-bin",it is a static content

    {
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        if (uri[strlen(uri) - 1] == '/') //end of uri is /, visit uri/home.html
        {
            strcat(filename, "home.html");
        }
        return 1;
    }
    else //dynamic content
    {
        ptr = index(uri, '?');
        if (ptr != NULL)
        {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        }
        else
        {
            strcpy(cgiargs, "");
        }
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}

void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp;
    char filetype[MAXLINE];
    char buf[MAXLINE];
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n", filesize);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: %s\r\n\r\n", filetype);
    Rio_writen(fd, buf, strlen(buf));

    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    Munmap(srcp, filesize);
}

void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
    {
        strcpy(filetype, "text/html");
    }
    else if (strstr(filename, ".gif"))
    {
        strcpy(filetype, "image/gif");
    }
    else if (strstr(filename, ".png"))
    {
        strcpy(filetype, "image/png");
    }
    else if (strstr(filename, ".jpg"))
    {
        strcpy(filetype, "image/jpeg");
    }
    else
    {
        strcpy(filetype, "text/plain");
    }
}

void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE];
    char *argv[] = {NULL};

    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if (Fork() == 0) // child process
    {
        setenv("QUERY_STRING", cgiargs, 1);
        Dup2(fd, STDOUT_FILENO);
        Execve(filename, argv, environ); //run CGI program
    }
    Wait(NULL);
}
