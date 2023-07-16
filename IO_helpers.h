
#ifndef IO_HELPERS_HEADER
#define IO_HELPERS_HEADER

#include <stdlib.h>

int32_t read_full(int fd, char *buf, size_t n);
int32_t write_all(int fd, char *buf, size_t n);

#endif