#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <cstdint>
#include <vector>
#include <poll.h>
#include "constants.h"
enum
{
    STATE_REQ = 0, // request
    STATE_RES = 1, // recieve
    STATE_END = 2, // delete
};

typedef struct Conn
{
    int fd = -1;
    uint32_t state = 0;

    // reading buffer
    uint8_t *rbuf[k_max_msg + HEADER];
    size_t rbuf_size = 0;

    // writing buffer
    uint8_t *wbuf[k_max_msg + HEADER];
    size_t wbuf_size = 0;
    size_t wbuf_sent = 0;
} Conn;

void event_loop(int fd, std::vector<Conn *> *clients, std::vector<struct pollfd> *queue);
void fd_set_nb(int fd);
void conn_put(std::vector<Conn *> &fd2conn, struct Conn *conn);
int32_t accept_new_conn(std::vector<Conn *> &fd2conn, int fd);
void connection_io(Conn *conn);
void state_req(Conn *conn);
void state_res(Conn *conn);
bool try_fill_buffer(Conn *conn);
bool try_one_request(Conn *conn);
bool try_flush_buffer(Conn *conn);
