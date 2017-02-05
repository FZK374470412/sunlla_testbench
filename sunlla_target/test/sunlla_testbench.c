/* Author	: Vedic.Fu
 * E-mail	: FZKmxcz@163.com
 * Copyright	: Free
 * Description	: main() function entry
 *		  1. Detect requestion come from client and create new
 *		     socket to connect with it circularly.
 *		  2. Create new process for new socket.
 */

#include "sunlla_common.h"
#include "sunlla_net.h"

#define PROCESS_SERVER_PORT	8888 /* Hope god will not use this port number*/

int sys_tick;

static void do_process(struct sunlla_trans *p_trans)
{
	char resp[64];
	int ret = 0;

	/* tell clinet that server is ready */
	sprintf(resp, "<resp><pid>%d</pid><status>ok</status></resp>", getpid());
	ret = sunlla_trans_sendxml(p_trans, resp, strlen(resp));
	if (ret < 0) {
		printf("%s,%d: sunlla_trans_sendxml error %d\n", __func__, __LINE__, ret);
		sunlla_trans_close(p_trans);
		free(p_trans);
		exit(-1);
	} else {
		sunlla_trans_loop(p_trans); /* will while(1) here until session end */
		sunlla_trans_close(p_trans);
		exit(0);
	}
}

static int creat_new_process(struct sunlla_trans *p_trans)
{
	int ret = 0;

	ret = fork();
	if (ret == 0) { /* child process */
		/* close server_fd??? */
		do_process(p_trans);
	} else if (ret > 0) { /* father process */
		sunlla_trans_close(p_trans); /* there are double p_trans's socket_fd after fork(), close it in father process, unless socket can not close after communication */
	} else {
		printf("%s,%d: fork error \n", __func__, __LINE__);
		return -1;
	}

	return 0;
}

int main(int argc, const char *argv[])
{
	int pid;
	int status;
	int server_fd = -1;
	fd_set rd_set, testfd;
	int fd;
	struct timeval timeval;
	struct sunlla_trans *p_trans;

	printf("\n===================================\n");
	printf("  	 SUNLLA TestBench\n");
	printf("===================================\n");

	sys_tick = sysconf(_SC_CLK_TCK);

	server_fd = sunlla_create_tcp(PROCESS_SERVER_PORT);
	if (server_fd < 0) {
		printf("%s,%d: cannot create process server socket\n", __func__, __LINE__);
		goto EXIT;
	}
	
	FD_ZERO(&rd_set);
	FD_SET(server_fd, &rd_set);
	while (1) {
		testfd = rd_set;
		/* timeout = 1s */
		timeval.tv_sec = 1;
		timeval.tv_usec = 0;
		
		if (select(FD_SETSIZE, &testfd, NULL, NULL, &timeval) > 0) {
			for (fd = 0; fd < FD_SETSIZE; fd++) {
				if (FD_ISSET(fd, &testfd) && fd == server_fd) {
					p_trans = sunlla_accept_tcp(server_fd);
					if (p_trans == NULL) {
						printf("%s,%d: process socket accept error \n", __func__, __LINE__);
						goto EXIT;
					}
				
					if (creat_new_process(p_trans) < 0) {
						printf("%s,%d: start_process error \n", __func__, __LINE__);
						goto EXIT;
					}
				}
			}
		}
		
		pid = waitpid(0, &status, WNOHANG);
		if(pid > 0) {
			if (WIFEXITED(status)) {
				printf("%s():%d: PID=%d exited normally -> %d\n", __func__, __LINE__, pid, WEXITSTATUS(status));
			} else {
				printf("%s():%d: PID=%d exited error\n", __func__, __LINE__, pid);
			}
		}
	}

EXIT:
	if (server_fd >= 0) {
		sunlla_stop_tcp(server_fd);
		server_fd = -1;
	}

	return 0;
}
