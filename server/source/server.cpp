#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <algorithm>
#include <netinet/ip.h>
#include <vector>
#include <map>

#include "../include/serialization.hpp"
#include "../include/avl.hpp"
#include "../include/linkedlist.hpp"
#include "../include/heap.hpp"
#include "../include/hashtable.hpp"

#define k_max_works 2000

#define container_of(ptr, type, member) ({                  \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type, member) ); })

static void msg(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg)
{
	int err = errno;
	fprintf(stderr, "[%d] %s\n", err, msg);
	abort();
}

static void fd_set_nb(int fd)
{
	errno = 0;
	int flags = fcntl(fd, F_GETFL, 0);
	if (errno)
	{
		die("fcntl error");
		return;
	}

	flags |= O_NONBLOCK;

	errno = 0;
	(void)fcntl(fd, F_SETFL, flags);
	if (errno)
	{
		die("fcntl error");
	}
}

std::string str_tolower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(),
				   // static_cast<int(*)(int)>(std::tolower)         // wrong
				   // [](int c){ return std::tolower(c); }           // wrong
				   // [](char c){ return std::tolower(c); }          // wrong
				   [](unsigned char c)
				   {
					   return std::tolower(c);
				   } // correct
	);
	return s;
}

const size_t k_max_msg = 4096;

enum
{
	STATE_REQ = 0,
	STATE_RES = 1,
	STATE_END = 2, // mark the connection for deletion
};

struct Conn
{
	int fd = -1;
	uint32_t state = 0; // either STATE_REQ or STATE_RES
	// buffer for reading
	size_t rbuf_size = 0;
	uint8_t rbuf[4 + k_max_msg];
	// buffer for writing
	size_t wbuf_size = 0;
	size_t wbuf_sent = 0;
	uint8_t wbuf[4 + k_max_msg];

	uint64_t idle_start = 0;
	// timer
	DList idle_list;
};

static struct
{
	HashTable db;

	// a map of all client connections, keyed by fd
	std::vector<Conn *> fd2conn;

	// timers for idle connections
	DList idle_list;

	// timers for TTLs
	Heap ttl_heap;
} g_data;

/* TIMERS */

const uint64_t k_idle_timeout_ms = 5 * 1000;

HashTable *g_map = initHashTable(2);
std::map<std::string, AVLTree *> avlTrees;
void evict_ANode(Heap_Node Anode)
{
	auto treeIter = avlTrees.find(Anode.get_Aname());
	if (treeIter == avlTrees.end())
	{
		// sorted set not found, do nothing
	}
	else
	{
		Node *node = treeIter->second->remove(treeIter->second->root, Anode.get_Akey());
	}
}
void evict_HNode(Heap_Node node)
{
	remove(g_map, node.get_Hkey());
}

static uint64_t get_monotonic_usec()
{
	timespec tv = {0, 0};
	clock_gettime(CLOCK_MONOTONIC, &tv);
	return uint64_t(tv.tv_sec) * 1000000 + tv.tv_nsec / 1000;
}

static uint32_t next_timer_ms()
{
	uint64_t now_us = get_monotonic_usec();
	uint64_t next_us = (uint64_t)-1;

	// idle timers
	if (!dlist_empty(&g_data.idle_list))
	{
		Conn *next = container_of(g_data.idle_list.next, Conn, idle_list);
		next_us = next->idle_start + k_idle_timeout_ms * 1000;
	}

	// ttl timers
	if (!g_data.ttl_heap.isEmpty() && g_data.ttl_heap.peek() < next_us)
	{
		next_us = g_data.ttl_heap.peek();
	}

	if (next_us == (uint64_t)-1)
	{
		return 10000; // no timer, the value doesn't matter
	}

	if (next_us <= now_us)
	{
		// missed?
		return 0;
	}

	return (uint32_t)((next_us - now_us) / 1000);
}

static void conn_done(Conn *conn)
{
	g_data.fd2conn[conn->fd] = NULL;
	(void)close(conn->fd);
	dlist_detach(&conn->idle_list);
	free(conn);
}

static void process_timers()
{
	uint64_t now_us = get_monotonic_usec();
	while (!dlist_empty(&g_data.idle_list))
	{
		Conn *next = container_of(g_data.idle_list.next, Conn, idle_list);
		uint64_t next_us = next->idle_start + k_idle_timeout_ms * 1000;
		if (next_us >= now_us + 1000)
		{
			// not ready, the extra 1000us is for the ms resolution of poll()
			break;
		}
		printf("removing idle connection: %d\n", next->fd);
		conn_done(next);
	}

	// TTL timers
	size_t nworks = 0;
	while (!g_data.ttl_heap.isEmpty() && g_data.ttl_heap.peek() < now_us)
	{
		// evict the peek timer and its corresponding node
		Heap_Node node = g_data.ttl_heap.poll();
		if (node.get_type())
		{
			evict_ANode(node);
		}
		else
		{
			evict_HNode(node);
		}
		if (nworks++ >= k_max_works)
		{
			// don't stall the server if too many keys are expiring at once
			break;
		}
	}
}

/* TIMERS */

static void conn_put(std::vector<Conn *> &fd2conn, struct Conn *conn)
{
	if (fd2conn.size() <= (size_t)conn->fd)
	{
		fd2conn.resize(conn->fd + 1);
	}
	fd2conn[conn->fd] = conn;
}

static int32_t accept_new_conn(std::vector<Conn *> &fd2conn, int fd)
{
	// accept
	struct sockaddr_in client_addr = {};
	socklen_t socklen = sizeof(client_addr);
	int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
	if (connfd < 0)
	{
		msg("accept() error");
		return -1; // error
	}

	// set the new connection fd to nonblocking mode
	fd_set_nb(connfd);
	// creating the struct Conn
	struct Conn *conn = (struct Conn *)malloc(sizeof(struct Conn));
	if (!conn)
	{
		close(connfd);
		return -1;
	}
	conn->fd = connfd;
	conn->state = STATE_REQ;
	conn->rbuf_size = 0;
	conn->wbuf_size = 0;
	conn->wbuf_sent = 0;
	conn->idle_start = get_monotonic_usec();
	dlist_insert_before(&g_data.idle_list, &conn->idle_list);
	conn_put(g_data.fd2conn, conn);
	return 0;
}

static void state_req(Conn *conn);

static void state_res(Conn *conn);

const size_t k_max_args = 1024;

/*
 parses the request in the buffer
 returns 0 if the request is parsed successfully
 returns -1 if the request is invalid
 */
static int32_t parse_req(const uint8_t *data, size_t len,
						 std::vector<std::string> &out)
{
	if (len < 4)
	{
		return -1;
	}
	uint32_t n = 0;
	memcpy(&n, &data[0], 4);
	if (n > k_max_args)
	{
		return -1;
	}

	size_t pos = 4;
	while (n--)
	{
		if (pos + 4 > len)
		{
			return -1;
		}
		uint32_t sz = 0;
		memcpy(&sz, &data[pos], 4);
		if (pos + 4 + sz > len)
		{
			return -1;
		}
		out.push_back(std::string((char *)&data[pos + 4], sz));
		pos += 4 + sz;
	}

	if (pos != len)
	{
		return -1; // trailing garbage
	}
	return 0;
}

enum
{
	RES_OK = 0,
	RES_ERR = 1,
	RES_NX = 2,
};

// The data structure for the key space. This is just a placeholder
// until we implement a hashtable in the next chapter.
// static std::map<std::string, std::string> g_map;

/*
 cmd:  [EXPIRE, (str)sorted_set_name, (int)key, (uint_64)number of seconds]	-> sorted set
		******************************** OR ********************************
 cmd:  [EXPIRE, (str)key, (uint_64)number of seconds]						-> HashTable
*/
static void do_expire(std::vector<std::string> &cmd, std::string &out)
{
	// 1- decide tree or hashtable
	// 2- get the data node's info
	// 3- add the key to the heap

	uint64_t ttl_val = stoi(cmd[2]);
	if (cmd.size() == 3)
	{
		// hashtable
		std::string value = get(g_map, cmd[1]);
		if (value == "")
		{
			return res_ser_err(out, RES_ERR, "Key not found");
		}
		else
		{
			g_data.ttl_heap.add(ttl_val, cmd[1]);
			res_ser_str(out, "TTL added successfully");
		}
	}
	else if (cmd.size() == 4)
	{
		// sorted set

		auto treeIter = avlTrees.find(cmd[1]);
		if (treeIter == avlTrees.end()) // sorted set not found
		{
			return res_ser_err(out, RES_ERR, "sorted set not found");
		}
		else
		{
			int rKey = stoi(cmd[2]);

			if (!treeIter->second->contains(treeIter->second->root, rKey))
			{
				return res_ser_err(out, RES_ERR, "Key not found");
			}
			g_data.ttl_heap.add(ttl_val, cmd[1], rKey);
			res_ser_str(out, "TTL added successfully");
		}
	}
}

/*
 cmd:  [Aset, list_name, Ahmed, 10]
*/
static void do_Aset(std::vector<std::string> &cmd, std::string &out)
{
	// (void)res;
	// (void)reslen;
	//	cmd[1] = str_tolower(cmd[1]);
	AVLTree *tree;
	const int key = stoi(cmd[3]);
	auto treeIter = avlTrees.find(cmd[1]);

	if (treeIter == avlTrees.end()) // new sorted set
	{
		tree = new AVLTree();
		tree->insert(tree->root, key, cmd[2]);
		avlTrees[cmd[1]] = tree;
	}
	else
	{
		tree = treeIter->second;

		if (tree->contains(tree->root, key))
		{
			return res_ser_str(out, "Already Exists!");
		}
		tree->insert(tree->root, key, cmd[2]);
	}
	return res_ser_nil(out);
}

/*
 cmd:  [Aget, list_name, 20]
 */
static void do_Aget(std::vector<std::string> &cmd, std::string &out)
{
	int rKey = stoi(cmd[2]);
	auto treeIter = avlTrees.find(cmd[1]);

	if (treeIter == avlTrees.end()) // sorted set not found
	{
		return res_ser_err(out, RES_ERR, "sorted set not found");
	}
	else
	{
		Node *node = treeIter->second->search(treeIter->second->root, rKey);

		if (node == NULL)
		{
			return res_ser_err(out, RES_ERR, "Key not found");
		}
		const std::string val = node->value;
		res_ser_str(out, val);
	}
}

/*
 cmd:  [adel, list_name, 10]
 */
static void do_Adel(std::vector<std::string> &cmd, std::string &out)
{
	// (void)res;
	// (void)reslen;
	// g_map.erase(cmd[1])
	auto treeIter = avlTrees.find(cmd[1]);
	if (treeIter == avlTrees.end()) // sorted set not found
	{
		return res_ser_err(out, RES_ERR, "sorted set not found");
	}
	else
	{
		// try
		// {
		int key = stoi(cmd[2]);
		Node *node = treeIter->second->remove(treeIter->second->root, key);
		// }
		// catch (const std::exception &e)
		// {
		// 	std::cout << e.what() << '\n';
		// }
		return res_ser_nil(out);
	}
}

static void do_get(std::vector<std::string> &cmd, std::string &out)
{
	cmd[1] = str_tolower(cmd[1]);
	if (!contains(g_map, cmd[1]))
	{
		return res_ser_err(out, RES_ERR, "Key not found");
	}
	const std::string val = get(g_map, cmd[1]);
	return res_ser_str(out, val);
}

static void do_set(std::vector<std::string> &cmd, std::string &out)
{
	// (void)res;
	// (void)reslen;
	cmd[1] = str_tolower(cmd[1]);
	set(g_map, cmd[1], cmd[2]);

	return res_ser_nil(out);
}

// cmd: ["del", "key"]
static void do_del(std::vector<std::string> &cmd, std::string &out)
{
	// (void)res;
	// (void)reslen;
	// g_map.erase(cmd[1])
	cmd[1] = str_tolower(cmd[1]);
	remove(g_map, cmd[1]);
	return res_ser_nil(out);
}

static void do_keys(std::vector<std::string> &cmd, std::string &out)
{
	res_ser_arr(out, (uint32_t)g_map->size);
	// traverse over the 1st hashtable
	for (int i = 0; i < g_map->tables[0].size; i++)
	{
		// go to the first table, traverse over the table
		// index into each linked list and str_out the key
		if (g_map->tables[0].table[i] != NULL)
		{
			size_t size = g_map->tables[0].table[i]->key.size();
			if (size != 0)
			{
				HNode *node = g_map->tables[0].table[i];
				while (node != NULL)
				{
					res_ser_str(out, node->key);
					node = node->next;
				}
			}
		}
	}

	// traverse over the 2nd hashtable
	for (int i = 0; i < g_map->tables[1].size; i++)
	{
		// go to the first table, traverse over the table
		// index into each linked list and str_out the key
		if (g_map->tables[1].table[i] != NULL)
		{
			size_t size = g_map->tables[1].table[i]->key.size();
			if (size != 0)
			{
				HNode *node = g_map->tables[1].table[i];
				while (node != NULL)
				{
					res_ser_str(out, node->key);
					node = node->next;
				}
			}
		}
	}
}

static bool cmd_is(const std::string &word, const char *cmd)
{
	return 0 == strcasecmp(word.c_str(), cmd);
}

static void do_request(std::vector<std::string> &cmd, std::string &out)
{

	if (cmd.size() == 1 && cmd_is(cmd[0], "keys"))
	{
		do_keys(cmd, out);
	}
	else if (cmd.size() == 2 && cmd_is(cmd[0], "get"))
	{
		do_get(cmd, out);
	}
	else if (cmd.size() == 3 && cmd_is(cmd[0], "set"))
	{
		do_set(cmd, out);
	}
	else if (cmd.size() == 2 && cmd_is(cmd[0], "del"))
	{
		do_del(cmd, out);
	}
	else if (cmd.size() == 4 && cmd_is(cmd[0], "aset"))
	{
		do_Aset(cmd, out);
	}
	else if (cmd.size() == 3 && cmd_is(cmd[0], "aget"))
	{
		do_Aget(cmd, out);
	}
	else if (cmd.size() == 3 && cmd_is(cmd[0], "adel"))
	{
		do_Adel(cmd, out);
	}
	else if (cmd.size() == 4 && cmd_is(cmd[0], "EXPIRE"))
	{
		do_expire(cmd, out);
	}
	else if (cmd.size() == 3 && cmd_is(cmd[0], "EXPIRE"))
	{
		do_expire(cmd, out);
	}
	else
	{
		// cmd is not recognized
		res_ser_err(out, RES_ERR, "Unknown Command");
	}
}

static bool try_one_request(Conn *conn)
{
	// try to parse a request from the buffer
	if (conn->rbuf_size < 4)
	{
		// not enough data in the buffer. Will retry in the next iteration
		return false;
	}

	uint32_t len = 0;
	memcpy(&len, &conn->rbuf[0], 4);

	if (len > k_max_msg)
	{
		msg("too long");
		conn->state = STATE_END;
		return false;
	}
	if (4 + len > conn->rbuf_size)
	{
		// not enough data in the buffer. Will retry in the next iteration
		return false;
	}

	// parse the request
	std::vector<std::string> cmd;
	int32_t err = parse_req(&conn->rbuf[4], len, cmd);
	if (err != 0)
	{
		msg("parse_req() error");
		conn->state = STATE_END;
		return false;
	}

	// TODO changes here
	//  got one request, generate the response in a string.
	std::string res;
	do_request(cmd, res);

	// check if the response is too long
	// length of data + 4 bytes for length
	if (res.size() + 4 > k_max_msg)
	{
		// clear the buffer to be able to send error msg
		res.clear();
		res_ser_err(res, RES_ERR, "too long");
	}

	/*
	 - add the length of the response to the response buffer res (4bytes)
	 - add the data of the response to the response buffer res   (length bytes)
	 - increase the write buffer size wbuf by 4 + wlen (length + data)
	 */
	uint32_t wlen = (uint32_t)res.size();
	memcpy(&conn->wbuf[0], &wlen, 4);
	memcpy(&conn->wbuf[4], res.data(), res.size());
	conn->wbuf_size = 4 + wlen;

	// remove the request from the buffer.
	// note: frequent memmove is inefficient.
	// note: need better handling for production code.
	size_t remain = conn->rbuf_size - 4 - len;
	if (remain)
	{
		memmove(conn->rbuf, &conn->rbuf[4 + len], remain);
	}
	conn->rbuf_size = remain;

	// change state
	conn->state = STATE_RES;
	state_res(conn);

	// continue the outer loop if the request was fully processed
	return (conn->state == STATE_REQ);
}

static bool try_fill_buffer(Conn *conn)
{
	// try to fill the buffer
	assert(conn->rbuf_size < sizeof(conn->rbuf));
	ssize_t rv = 0;
	do
	{
		size_t cap = sizeof(conn->rbuf) - conn->rbuf_size;
		rv = read(conn->fd, &conn->rbuf[conn->rbuf_size], cap);
	} while (rv < 0 && errno == EINTR);
	if (rv < 0 && errno == EAGAIN)
	{
		// got EAGAIN, stop.
		return false;
	}
	if (rv < 0)
	{
		msg("read() error");
		conn->state = STATE_END;
		return false;
	}
	if (rv == 0)
	{
		if (conn->rbuf_size > 0)
		{
			msg("unexpected EOF");
		}
		else
		{
			msg("EOF");
		}
		conn->state = STATE_END;
		return false;
	}

	conn->rbuf_size += (size_t)rv;
	assert(conn->rbuf_size <= sizeof(conn->rbuf));

	// Try to process requests one by one.
	// Why is there a loop? Please read the explanation of "pipelining".
	while (try_one_request(conn))
	{
	}
	return (conn->state == STATE_REQ);
}

static void state_req(Conn *conn)
{
	while (try_fill_buffer(conn))
	{
	}
}

static bool try_flush_buffer(Conn *conn)
{
	ssize_t rv = 0;
	do
	{
		size_t remain = conn->wbuf_size - conn->wbuf_sent;
		rv = write(conn->fd, &conn->wbuf[conn->wbuf_sent], remain);
	} while (rv < 0 && errno == EINTR);
	if (rv < 0 && errno == EAGAIN)
	{
		// got EAGAIN, stop.
		return false;
	}
	if (rv < 0)
	{
		msg("write() error");
		conn->state = STATE_END;
		return false;
	}
	conn->wbuf_sent += (size_t)rv;
	assert(conn->wbuf_sent <= conn->wbuf_size);
	if (conn->wbuf_sent == conn->wbuf_size)
	{
		// response was fully sent, change state back
		conn->state = STATE_REQ;
		conn->wbuf_sent = 0;
		conn->wbuf_size = 0;
		return false;
	}
	// still got some data in wbuf, could try to write again
	return true;
}

static void state_res(Conn *conn)
{
	while (try_flush_buffer(conn))
	{
	}
}

static void connection_io(Conn *conn)
{

	// waked up by poll, update the idle timer
	// by moving conn to the end of the list.
	conn->idle_start = get_monotonic_usec();
	dlist_detach(&conn->idle_list);
	dlist_insert_before(&g_data.idle_list, &conn->idle_list);

	if (conn->state == STATE_REQ)
	{
		state_req(conn);
	}
	else if (conn->state == STATE_RES)
	{
		state_res(conn);
	}
	else
	{
		assert(0); // not expected
	}
}

int main()
{

	dlist_init(&g_data.idle_list);
	g_data.ttl_heap = Heap(); // initialize ttl-heap

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		die("socket()");
	}

	int val = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

	// bind
	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = ntohs(1234);
	addr.sin_addr.s_addr = ntohl(0); // wildcard address 0.0.0.0
	int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
	if (rv)
	{
		die("bind()");
	}

	// listen
	rv = listen(fd, SOMAXCONN);
	if (rv)
	{
		die("listen()");
	}

	// a map of all client connections, keyed by fd
	std::vector<Conn *> fd2conn;

	// set the listen fd to nonblocking mode
	fd_set_nb(fd);

	// the event loop
	std::vector<struct pollfd> poll_args;
	while (true)
	{
		// prepare the arguments of the poll()
		poll_args.clear();
		// for convenience, the listening fd is put in the first position
		struct pollfd pfd = {fd, POLLIN, 0};
		poll_args.push_back(pfd);
		// connection fds
		for (size_t i = 1; i < fd2conn.size(); ++i)
		{

			Conn *conn = fd2conn[i];

			if (!conn)
			{
				continue;
			}
			struct pollfd pfd = {};
			pfd.fd = conn->fd;
			pfd.events = (conn->state == STATE_REQ) ? POLLIN : POLLOUT;
			pfd.events = pfd.events | POLLERR;
			poll_args.push_back(pfd);
		}

		// poll for active fds
		int timeout_ms = (int)next_timer_ms();
		int rv = poll(poll_args.data(), (nfds_t)poll_args.size(), timeout_ms);
		if (rv < 0)
		{
			die("poll");
		}

		// process active connections
		for (size_t i = 1; i < poll_args.size(); ++i)
		{
			if (poll_args[i].revents)
			{
				Conn *conn = g_data.fd2conn[poll_args[i].fd];
				connection_io(conn);
				if (conn->state == STATE_END)
				{
					// client closed normally, or something bad happened.
					// destroy this connection
					// fd2conn[conn->fd] = NULL;
					//(void)close(conn->fd);
					// free(conn);
					conn_done(conn);
				}
			}
		}

		// handle timers
		process_timers();

		// try to accept a new connection if the listening fd is active
		if (poll_args[0].revents)
		{
			(void)accept_new_conn(fd2conn, fd);
		}
	}

	return 0;
}

