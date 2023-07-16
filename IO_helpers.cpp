#include <assert.h>
#include <unistd.h>
#include <stdlib.h>

int32_t read_full(int fd, char *buf, size_t n)
{

    while (n > 0)
    {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0)
        {
            return -1;
        }
        assert((size_t)rv <= n); // used for diagnostics
        n -= (size_t)rv;
        buf += rv;
    }
    return 0; // success
}

int32_t write_all(int fd, char *buf, size_t n)
{
    while (n > 0)
    {
        ssize_t wv = write(fd, buf, n);
        if (wv <= 0)
        {
            return -1; // wrote nothing
        }
        assert((ssize_t)wv <= n);
        n -= (ssize_t)wv;
        buf += wv;
    }
    return 0;
}