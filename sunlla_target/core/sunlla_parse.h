#ifndef __SUNLLA_PARSE_H__
#define __SUNLLA_PARSE_H__

int sunlla_parse_xml(const char *content, int length, int *pcmd, int *attrs, void **params);

int sunlla_exe_cmd(int cmd, int *in_attrs, void **in_params, int *out_attrs, void **out_params);

int sunlla_wrap_xml(int cmd, int ret, int *attrs, void **params,  char *xml_data, int xml_len);

#endif /* __SUNLLA_PARSE_H__ */
