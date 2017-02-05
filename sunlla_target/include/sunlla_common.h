#ifndef __SUNLLA_COMMON_H__
#define __SUNLLA_COMMON_H__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/times.h>

#define uchar unsigned char
#define uint unsigned int

#define RESP_BUF_SIZE	256
#define ATTR_NUM_MAX	20
#define END_OF_ATTR	-1

extern int sys_tick;

#endif /* __SUNLLA_COMMON_H__ */
