/* Author	: Vedic.Fu
 * E-mail	: FZKmxcz@163.com
 * Copyright	: Free
 * Description	: Simple test for I2C module
 */

#include "sunlla_common.h"
#include "sunlla_unify.h"


static int sunlla_i2c_open(int dev)
{
	printf("%s()..... dev=%d\n", __func__, dev);
	return 0;
}

static int sunlla_i2c_start(int dev)
{
	printf("%s()..... dev=%d\n", __func__, dev);
	return 0;
}

static int sunlla_i2c_close(int dev, int *checbak)
{
	printf("%s()..... dev=%d\n", __func__, dev);
	*checbak = 93;
	return 0;
}


int f_sunlla_i2c_test(int cmd, int in_attrs[], void *in_param[], int out_attrs[], void *out_param[])
{
	int ret;
	int *attr = in_attrs;
	void **param = in_param;
	int dev=-1;
	int checbak=0;
	char buf[16]={0};
	
	out_attrs[0] = END_OF_ATTR;
	while (*attr != END_OF_ATTR) {
		switch (*attr) {
			case ATTR_I2C_dev:
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
	case CMD_I2C_OPEN:			
		ret = (0 == sunlla_i2c_open(dev)) ? 0 : 1;
		break;
		
	case CMD_I2C_START:
		ret = (0 == sunlla_i2c_start(dev)) ? 0 : 1;
		break;
		
	case CMD_I2C_CLOSE:		
		ret = (0 == sunlla_i2c_close(dev, &checbak)) ? 0 : 1;
		out_attrs[0] = ATTR_I2C_checbak;
		sprintf(buf, "%d", checbak); /* transmit must as string */
		out_param[0] = strdup(buf); /* after transmit will free out_param, however buf in stack memery, just copy once more */
		out_attrs[1] = END_OF_ATTR;
		break;

	
	default:
		printf("%s: invalid command %s\n", __func__, cmd_name[cmd]);
		return 1;
	}
	
	return ret;
}
