#ifndef __SUNLLA_TRANS_H__
#define __SUNLLA_TRANS_H__

enum trans_type {
	TRANS_INVALID     = -1,
	TRANS_RESP        = 0,
	TRANS_XML         = 1,
	TRANS_EXIT        = 2
};

enum trans_err_type {
	TRANS_TIMEOUT   = -1,
	BAD_CHECKCRC    = -2,
	INVALID_DATA    = -3,
	SHOULD_CLOSE    = -4,
	RTN_SUCCESS	= 0
};

struct sunlla_trans {
	int sockfd;

	int (*close)(int sockfd);
	void (*flush)(int sockfd);
	int (*write)(int sockfd, const char *data, int len, int ms);
	int (*read)(int sockfd, char *data, int len, int ms);
};


int sunlla_trans_close(struct sunlla_trans *p_trans);

int sunlla_trans_rcevxml(struct sunlla_trans *trans, char *data, int len, int *p_cmd);

int sunlla_trans_sendxml(struct sunlla_trans *trans, char *data, int len);

void sunlla_trans_loop(struct sunlla_trans *p_trans);

#endif /* __SUNLLA_TRANS_H__ */
