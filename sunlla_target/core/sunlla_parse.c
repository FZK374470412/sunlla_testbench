/* Author	: Vedic.Fu
 * E-mail	: FZKmxcz@163.com
 * Copyright	: Free
 * Description	: parse/wrap data format -- XML
 */

#include "sunlla_common.h"
#include "sunlla_unify.h"
#include "sunlla_parse.h"
#include "mxml.h"

int sunlla_exe_cmd(int cmd, int *in_attrs, void **in_params, int *out_attrs, void **out_params)
{
	int module = cmd_array[cmd].module;
	return module_array[module].func(cmd, in_attrs, in_params, out_attrs, out_params);
}


int sunlla_parse_xml(const char *xml, int length, int *pcmd, int *attrs, void **params)
{
	int result = -1;
	int cmd = -1;
	int idx = 0;
	int attr_cnt = 0;
	mxml_node_t *root, *cmd_node, *sub_node;
	char *cmd_op = NULL;
	int *attr  = attrs;
	void **param = params;
	*attr = END_OF_ATTR;
	(void)length;

	root = mxmlLoadString(NULL, xml, MXML_NO_CALLBACK);

	if (strcasecmp("test", mxmlGetElement(root))) {
		printf("the root name should be test\n");
		goto EXIT;
	}


	cmd_node = mxmlFindElement(root, root, "cmd", NULL, NULL, MXML_DESCEND);
	if (NULL == cmd_node) {
		printf("can not find cmd lable!\n");
		goto EXIT;
	}
	cmd_op = strdup(cmd_node->child->value.text.string);
	mxmlDelete(cmd_node);
	//printf("find cmd = %s\n", cmd_op);

	for (cmd = 0; cmd < NUM_OF_CMD; cmd++) {
		if (!strcmp(cmd_op, cmd_name[cmd]))
		    break;
	}
	if (NUM_OF_CMD == cmd) {
		printf("cmd %s not found\n", cmd_op);
		goto EXIT;
	} else {
		*pcmd = cmd;
	}

	for (sub_node=mxmlGetFirstChild(root); sub_node; sub_node=mxmlGetNextSibling(sub_node)) {
		const char *key = mxmlGetElement(sub_node);
		const char *value = mxmlGetText(sub_node, NULL);

		for (idx = 0; idx < NUM_OF_ATTR; idx++) {
			if (!strcmp(key, attr_name[idx]))
			    break;
		}

		if (NUM_OF_ATTR == idx) {
		    printf("xml attr %s not found!\n", key);
		    goto EXIT;
		}

		*param++ = strdup(value);	
		*attr++ = idx;	
		*attr = END_OF_ATTR;
		if (++attr_cnt > ATTR_NUM_MAX) {
			printf("too many parameters %d\n", attr_cnt);
			goto EXIT;
		}
	}

	if (attr_cnt > cmd_array[cmd].in) {
		printf("input marameters should within %d, attr_cnt = %d\n", cmd_array[cmd].in, attr_cnt);
		goto EXIT;
	}

	result = 0;

EXIT:
	if (root != NULL) {
		mxmlDelete(root);
	}
	if (cmd_op !=NULL)
		free(cmd_op);

	return result;
}

int sunlla_wrap_xml(int cmd, int result, int *attrs, void **params,  char *xml_data, int xml_len)
{
	int ret = -1;
	int idx = 0;
	int attr_cnt = 0;
	int total_size = 0;

	int *attr = attrs;
	void **param = params;

	mxml_node_t *root, *node;

	if (!params || !attrs || !xml_data) {
		printf("invalid input parameters\n");
		return ret;
	}

	root = mxmlLoadString(NULL, "<resp></resp>", MXML_NO_CALLBACK);
	if (NULL == root) {
		printf("mxmlLoadString fail!\n");
		return ret;
	}
	
	const char *status = (0 == result) ? "ok" : "error";
	node = mxmlNewElement(root, "status");
	mxmlNewText(node, 0, status);	

	while (*attr != END_OF_ATTR) {
		node = mxmlNewElement(root, attr_name[*attr]);
		mxmlNewText(node, 0, *param);

		param++;
		attr++;
		if (++attr_cnt > cmd_array[cmd].out) {
			printf("there are most %d output parameters, received %d\n", cmd_array[cmd].out, attr_cnt);
			goto EXIT;
		}
	}

	total_size = mxmlSaveString(root, xml_data, xml_len, MXML_NO_CALLBACK);
	if (total_size > xml_len) {
		printf("xml_len = %d too small!\n", xml_len);
		goto EXIT;
	}

	ret = 0;
	printf("response string: %s\n", xml_data);

EXIT:
	if (root != NULL)
		mxmlDelete(root);

	return ret;
}
