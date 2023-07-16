#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "server.h"

int main()
{
    printf("in server\n");
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0); // wildcard address 0.0.0.0
    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv)
    {
        perror("rv server");
        close(fd);
        exit(1);
    }
    listen(fd, SOMAXCONN);

    if (rv)
    {
        perror("r2 server");
        close(fd);
        exit(1);
    }

    while (true)
    {
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
        if (connfd <= 0)
        {
            perror("connfd in server");
            continue;
        }
        while (true)
        {
            int32_t err = one_request(connfd);
            if (err)
            {
                break;
            }
        }
        close(connfd);
        // do_something(connfd);
    }
}

// static void do_something(int connfd)
// {
//     char rbuf[64] = {};
//     printf("in do_something\n");
//     ssize_t data = read(connfd, rbuf, sizeof(rbuf) - 1);
//     printf("read: %s\n", rbuf);
//     if (data < 0)
//     {
//         perror("read() error in server");
//         return;
//     }
//     printf("client says: %s\n", rbuf);
//     char wbuf[] = "world";
//     write(connfd, wbuf, strlen(wbuf));
// }

