#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main()
{
    printf("in client\n");
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("client socket creation failed");
        close(fd);
        exit(1);
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    int rv = connect(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv)
    {
        perror("connection error in client");
        close(fd);
        exit(1);
    }

    char msg[] = "hello";
    write(fd, msg, strlen(msg));

    char rbuf[64] = {};
    ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
    if (n < 0)
    {
        perror("read");
        close(fd);
        exit(1);
    }
    printf("server says: %s\n", rbuf);
    close(fd);
}