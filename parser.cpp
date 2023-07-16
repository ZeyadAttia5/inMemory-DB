#include "IO_helpers.h"
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <cstdint>

#define HEADER 4
const size_t k_max_msg = 4096;

/*

    protocol::
        +-----+------+-----+------+--------
        | len | msg1 | len | msg2 | more...
        +-----+------+-----+------+--------


    Frame::

         Header  Message
        +-------+--------+
        | len   | msg    |
        +-------+--------+
         4bytes   k_max_msg bytes

    1) read the Header to determine the length of msg1
    2) read msg1 into the reading-buffer (rbuf)
    3) terminate rbuf

    4) Prepare to reply (instantiate writing-buffer wbuf) and the reply message
    5) write the length of the reply into the Header of wbuf
    6) copy the reply message into the wbuf
    7) send wbuf
*/

/*
 *   one_request function parses only one request and responds,
 *   until something bad happens or the client connection is lost
 */
int32_t one_request(int connfd)
{
    char rbuf[HEADER + k_max_msg + 1]; // +1 for the null terminator
    errno = 0;
    int32_t err = read_full(connfd, rbuf, HEADER);
    if (err)
    {
        if (errno == 0)
        {
            printf("EOF\n\n");
            return -1;
        }
        else
        {
            perror("error reading the header");
        }
    }

    // get the length of the message
    uint32_t len = 0;
    memcpy(&len, rbuf, HEADER);
    if (len > k_max_msg)
    {
        printf("length of message = %i\n", len);
        printf("maximum length of message = %li\n", k_max_msg);
        perror("the message is too long");
        return -1;
    }

    // request the message
    err = read_full(connfd, rbuf + HEADER, len);
    if (err)
    { // if not zero => error
        perror("error reading the message");
        return -1;
    }

    // terminate the buffer
    rbuf[HEADER + len] = '\0';
    printf("client says: %s\n", &rbuf[4]);

    // reply using the same protocol
    const char reply[] = "world!\n";
    char wbuf[HEADER + sizeof(reply)];
    len = (uint32_t)strlen(reply);
    memcpy(wbuf, &len, HEADER);
    memcpy(wbuf + HEADER, reply, len);

    return write_all(connfd, wbuf, len + HEADER);
}