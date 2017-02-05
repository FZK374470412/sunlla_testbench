/* Author	: Vedic.Fu
 * E-mail	: FZKmxcz@163.com
 * Copyright	: Free
 * Description	: transmit operation wrapper
 */

#include "sunlla_common.h"
#include "sunlla_trans.h"
#include "sunlla_parse.h"

#define CMD_IDX		(0)
#define DAT_IDX		(1)
#define CRC_LEN		(1)
#define HEADER_LEN	(5)
#define TIMEOUT_SHORT	(200)
#define TIMEOUT_LONG	(30000)

#define MAX_CMD_BUF	2048
#define DEF_TIMEOUT	3

char *resp_error = "<resp><status>error</status></resp>";
char *resp_ok = "<resp><status>ok</status></resp>";

static uchar check_crc(uchar chek_org, uchar *data, int len)
{
	int i = 0;

	for (i = 0; i < len; i++) {
		chek_org ^= data[i];
	}
	
	return chek_org;
}

static void u32_to_seq(uint x, char *seq)
{
	seq[0] = (char)((x >> 24) & 0xFF);
	seq[1] = (char)((x >> 16) & 0xFF);
	seq[2] = (char)((x >> 8) & 0xFF);
	seq[3] = (char)(x & 0xFF);
}

static uint seq_to_u32(uchar *seq)
{
	return (seq[0] << 24) + (seq[1] << 16) + (seq[2] << 8) + seq[3];
}

/* before transmit real data, we need tell reveiver the data type and how
 * many data will be transmited. So we build a header as format:
 * [cmd_type] + [data_len]
 * cmd_type: prefer to enum trans_cmd_type, one byte store
 * data_len: 4 byte store
 */
static void build_header(char *header, char cmd_type, uint data_len)
{
	header[CMD_IDX] = cmd_type;
	u32_to_seq(data_len, &header[DAT_IDX]);
}

static uint get_len(char *header)
{
	return seq_to_u32((header + DAT_IDX));
}


int sunlla_trans_close(struct sunlla_trans *p_trans)
{
	return p_trans->close(p_trans->sockfd);
}

int sunlla_trans_rcevxml(struct sunlla_trans *trans, char *xml, int max_len, int *cmd_type)
{
	char header[HEADER_LEN] = {0};
	char crc = '\0';
	int ret = 0;
	unsigned int xml_len = 0;
	unsigned int data_len = 0;

	/* wait for correct header receiving*/
	while(!ret) {
		ret = trans->read(trans->sockfd, header, HEADER_LEN, TIMEOUT_SHORT);
		if (ret == TRANS_TIMEOUT)
			trans->flush(trans->sockfd);
		else if (ret == SHOULD_CLOSE)
			return SHOULD_CLOSE;
		else
			ret = 1;
	}

	/* the first byte of header is command type! */
	*cmd_type = header[CMD_IDX];
	data_len = get_len(header);
	crc = check_crc(0, (uchar *)header, HEADER_LEN);


	xml_len = data_len + CRC_LEN;
	if (xml_len > (unsigned int)max_len) {
		printf("command too long %u, max length is: %d\n", xml_len, max_len);
		trans->flush(trans->sockfd);
		return INVALID_DATA;
	}

	ret = trans->read(trans->sockfd, xml, xml_len, TIMEOUT_LONG);
	if (ret != RTN_SUCCESS) {
		printf("receive real data error ret=%d\n", ret);
		return SHOULD_CLOSE;
	}

	crc = check_crc(crc, (uchar *)xml, xml_len - CRC_LEN);
	if (crc != xml[xml_len - CRC_LEN]) {
		printf("bad cmd check_crc, expect 0x%02x but got 0x%02x\n", xml[xml_len - CRC_LEN], crc);
		return BAD_CHECKCRC;
	}

	if (*cmd_type == TRANS_XML) {
		xml[xml_len - CRC_LEN] = '\0';
	}

	return data_len;
}

int sunlla_trans_sendxml(struct sunlla_trans *trans, char *xml, int data_len)
{
	char header[HEADER_LEN] = { 0 };
	int ret = 0;
	char crc = '\0';

	build_header(header, TRANS_RESP, data_len);
	ret = trans->write(trans->sockfd, header, HEADER_LEN, TIMEOUT_LONG);
	if (ret != RTN_SUCCESS) {
		printf("send header error, ret: %d\n", ret);
		return SHOULD_CLOSE;
	}

	crc = check_crc(0, (uchar *)header, HEADER_LEN);
	if (data_len > 0) {
		ret = trans->write(trans->sockfd, xml, data_len, TIMEOUT_LONG);
		if (ret != RTN_SUCCESS) {
			printf("send response data error, ret: %d\n", ret);
			return SHOULD_CLOSE;
		}
		crc = check_crc(crc, (uchar *)xml, data_len);
	}

	ret = trans->write(trans->sockfd, &crc, CRC_LEN, TIMEOUT_LONG);
	if (ret != RTN_SUCCESS) {
		return SHOULD_CLOSE;
	}

	return 0;
}

static void free_params(int *attrs, void **params)
{
    int i = 0;
    if (NULL == attrs || NULL == params)
        return;

    while (attrs[i] != END_OF_ATTR) {
        free(*(params + i));
        ++i;
    }
}

static int handle_xml_cmd(struct sunlla_trans *p_trans, char *xml, int len)
{
	int   out_attrs[ATTR_NUM_MAX + 1] = { 0 };
	void *out_params[ATTR_NUM_MAX + 1] = { NULL };
	int   in_attrs[ATTR_NUM_MAX + 1]   = { 0 };
	void *in_params[ATTR_NUM_MAX]       = { NULL };

	int cmd = TRANS_INVALID;
	int result = 0;
	char resp_buf[RESP_BUF_SIZE] = {0};

	printf("receive xml - pid:%d: \n%s\n", getpid(), xml);
	if (sunlla_parse_xml(xml, len, &cmd, in_attrs, in_params)) {
		printf("xml parsing error\n");
		free_params(in_attrs, in_params);
		return sunlla_trans_sendxml(p_trans, resp_error, strlen(resp_error));
	}


	result = sunlla_exe_cmd(cmd, in_attrs, in_params, out_attrs, out_params);
	free_params(in_attrs, in_params);

	result = sunlla_wrap_xml(cmd, result, out_attrs, out_params, resp_buf, RESP_BUF_SIZE);
	if (result) {
		printf("create result string error\n");
		return sunlla_trans_sendxml(p_trans, resp_error, strlen(resp_error));
	}

	result = sunlla_trans_sendxml(p_trans, resp_buf, strlen(resp_buf));
	free_params(out_attrs, out_params); /* free params after transmitted*/

	return result;
}

static int handle_command(struct sunlla_trans *p_trans, int cmd, char *xml, int len)
{
	if (TRANS_XML == cmd) {
		return handle_xml_cmd(p_trans, xml, len);
	}

	return sunlla_trans_sendxml(p_trans, resp_error, strlen(resp_error));
}

void sunlla_trans_loop(struct sunlla_trans *p_trans)
{
	int command = TRANS_INVALID;
	char buffer[MAX_CMD_BUF + 1] = { 0 };
	int quit = 0;

	while (!quit) {
		memset(buffer, 0, MAX_CMD_BUF + 1);
		int ret = sunlla_trans_rcevxml(p_trans, buffer, MAX_CMD_BUF, &command);

		switch (ret) {
		case SHOULD_CLOSE:
			printf("close current session!\n");
			quit = 1;
			break;

		case INVALID_DATA:
		case BAD_CHECKCRC:
			if (SHOULD_CLOSE == sunlla_trans_sendxml(p_trans, resp_error, sizeof(resp_error))) {
				printf("send error response error.\n");
				quit = 1;
			}
			break;

		case TRANS_TIMEOUT:
			break;

		default:
			if (SHOULD_CLOSE == handle_command(p_trans, command, buffer, ret)) {
				printf("handle command error.\n");
				quit = 1;
			}
			break;
		}
	}
}

