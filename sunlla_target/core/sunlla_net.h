#ifndef __SUNLLA_NET_H__
#define __SUNLLA_NET_H__

#include "sunlla_trans.h"


int sunlla_create_tcp(short port);
struct sunlla_trans *sunlla_accept_tcp(int server_fd);
int sunlla_stop_tcp(int server_fd);

#endif /* __SUNLLA_NET_H__ */
