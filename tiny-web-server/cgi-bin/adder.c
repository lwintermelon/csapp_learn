#include "csapp.h"
int main(int argc, char const *argv[])
{
    char *buf;
    char *p;
    char arg1[MAXLINE];
    char arg2[MAXLINE];
    char content[MAXLINE];
    int n1 = 0;
    int n2 = 0;

    if ((buf = getenv("QUERY_STRING")) != NULL)
    {
        p = strchr(buf, '&');
        *p = '\0';
        strcpy(arg2, p + 1);
        strcpy(arg1, buf);
        n1 = atoi(arg1);
        n2 = atoi(arg2);
    }

    // generate response body
    sprintf(content, "Welcom to CGI add: \r\n");
    sprintf(content, "%sThe answer is: %d + %d = %d\r\n", content, n1, n2, n1 + n2);

    printf("Connection: close\r\n");
    printf("Content-length %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");

    printf("%s", content);
    fflush(stdout);
    exit(0);
}
