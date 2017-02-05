/* Author	: Vedic.Fu
 * E-mail	: FZKmxcz@163.com
 * Copyright	: Free
 * Description	: Simple test for I2S module
 */
#include "sunlla_common.h"
#include "sunlla_unify.h"


static int sunlla_i2s_open(int dev)
{
	printf("%s()..... dev=%d\n", __func__, dev);
	return 0;
}

static int sunlla_i2s_start(int dev)
{
	printf("%s()..... dev=%d\n", __func__, dev);
	return 0;
}

static int sunlla_i2s_close(int dev)
{
	printf("%s()..... dev=%d\n", __func__, dev);
	return 0;
}



/*
 * Entry function
 */
int f_sunlla_i2s_test(int cmd, int in_attrs[], void *in_param[],
		    int out_attrs[], void *out_param[])
{
	int ret;
	int *attr = in_attrs;
	void **param = in_param;
	int dev=-1;
	
	out_attrs[0] = END_OF_ATTR;
	while (*attr != END_OF_ATTR) {
		switch (*attr) {
			case ATTR_I2S_dev:
				dev = atoi(*param);
				break;
			default:
				printf("%s error attr %s by cmd %s\n", __func__, attr_name[*attr], cmd_name[cmd]);
				return -1;
		}
		attr++;
		param++;
	}
	
	switch (cmd) {
	case CMD_I2S_OPEN:			
		ret = (0 == sunlla_i2s_open(dev)) ? 0 : 1;
		break;
		
	case CMD_I2S_START:
		ret = (0 == sunlla_i2s_start(dev)) ? 0 : 1;
		break;
		
	case CMD_I2S_CLOSE:		
		ret = (0 == sunlla_i2s_close(dev)) ? 0 : 1;
		break;

	
	default:
		printf("%s: invalid command %s\n", __func__, cmd_name[cmd]);
		return 1;
	}
	
	return ret;
}
