/* Author	: Vedic.Fu
 * E-mail	: FZKmxcz@163.com
 * Copyright	: Free
 * Description	: network functions of socket
 */

#include "sunlla_common.h"
#include <netinet/in.h>
#include "sunlla_net.h"


static int __net_read(int sock_fd, char *data, int len, int ms)
{
	int offset = 0;
	int read_cnt = len;
	int read_tmp = 0;
	int tick_end = times(NULL) + ms * sys_tick / 1000; /* current ticks + should run ticks */

	do {
		read_tmp = read(sock_fd, data + offset, read_cnt);
		if (read_tmp > 0) {
			offset += read_tmp;
			read_cnt -= read_tmp;
		} else if (read_tmp == 0) {
			printf("there is not more command, unlink....\n");
			return SHOULD_CLOSE;
		} else { /* operation fail */
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
				usleep(1000); /* client don't ready to send data */
			} else {
				printf("%s():%d read error! read_tmp=%d, errno=%d\n", __func__, __LINE__, read_tmp, errno);
				return SHOULD_CLOSE;
			}
		}

	} while (read_cnt > 0 && times(NULL) < tick_end);

	if (read_cnt != 0) {
		printf("%s():%d time out!\n", __func__, __LINE__);
		return TRANS_TIMEOUT;
	}

	return RTN_SUCCESS;
}

static int __net_write(int sock_fd, const char *data, int len, int ms)
{
	int offset = 0;
	int write_cnt = len;
	int write_tmp = 0;
	int tick_end = times(NULL) + ms * sys_tick / 1000; /* current ticks + should run ticks */

	do {
		write_tmp = write(sock_fd, data + offset, write_cnt);
		if (write_tmp > 0) {
			offset += write_tmp;
			write_cnt -= write_tmp;
		} else if (write_tmp == 0) {
			printf("there is not more data to write, unlink....\n");
			return SHOULD_CLOSE;
		} else { /* operation fail */
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
				usleep(1000); /* client don't ready to receive data */
			} else {
				printf("%s():%d write error! read_tmp=%d, errno=%d\n", __func__, __LINE__, write_tmp, errno);
				return SHOULD_CLOSE;
			}
		}

	} while (write_cnt > 0 && times(NULL) < tick_end);

	if (write_cnt != 0) {
		printf("%s():%d time out!\n", __func__, __LINE__);
		return TRANS_TIMEOUT;
	}

	return RTN_SUCCESS;
}

static void __net_flush(int sock_fd)
{
	char buffer[100];

	while(read(sock_fd, buffer, sizeof(buffer)) > 0)
		usleep(1000);
}

static int __net_close(int sock_fd) {
	return close(sock_fd);
}

/* ===================== TCP functions ====================
 * The interface for applications
 */

int sunlla_create_tcp(short port)
{
	int server_sockfd, server_len;	
	int so_reuseaddr = 1;
	struct sockaddr_in server_address;

	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == server_sockfd) {
		printf("cannot create socket!\n");
		return -1;
	}

	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(port);
	server_len = sizeof(server_address);

	setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&so_reuseaddr, sizeof(so_reuseaddr));
	
	if (-1 == bind(server_sockfd, (struct sockaddr *)&server_address, server_len)) {
		printf("bind socket error!\n");
		close(server_sockfd);
		return -1;
	}


	if (-1 == listen(server_sockfd, 5)) {
		printf("listen socket error!\n");
		close(server_sockfd);
		return -1;
	}

	return server_sockfd;
}

struct sunlla_trans *sunlla_accept_tcp(int server_sockfd)
{
	int client_sockfd = -1;
	struct sunlla_trans *p_trans;
	struct sockaddr_in client_address;
	socklen_t client_len = sizeof(client_address);

	client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
	if (client_sockfd < 0) {
		printf("socket accept error.\n");
		return NULL;
	}

	p_trans = malloc(sizeof(struct sunlla_trans));
	if(p_trans == NULL) {
		printf("malloc sunlla_trans fail!\n");
		close(client_sockfd);
		return NULL;
	}
	
	fcntl(client_sockfd, F_SETFL, fcntl(client_sockfd, F_GETFL, 0) | O_NONBLOCK);

	p_trans->sockfd = client_sockfd;
	p_trans->close = __net_close;
	p_trans->flush = __net_flush;
	p_trans->read = __net_read;
	p_trans->write = __net_write;

	return p_trans;
}

int sunlla_stop_tcp(int server_sockfd) {
	return close(server_sockfd);
}

