#ifndef __SUNLLA_UNIFY_H__
#define __SUNLLA_UNIFY_H__

#define MODULE(module, func)
#define CMD(module, op, in, out)
#define ATTR(module, attr)

/* Module enumeration */
#undef MODULE
#define MODULE(module, func) MODULE_##module,
enum module_list {
	#include <sunlla_all.h>
	NUM_OF_MODULE
};
#undef MODULE
#define MODULE(module, func)	

/* Command enumeration */
#undef CMD
#define CMD(module, op, in, out) CMD_##module##_##op,
enum cmd_list {
	#include <sunlla_all.h>
	NUM_OF_CMD
};
#undef CMD
#define CMD(module, op, in, out)

/* Attribute enumeration */
#undef ATTR
#define ATTR(module, attr) ATTR_##module##_##attr,
enum attr_list {
	#include <sunlla_all.h>
	NUM_OF_ATTR
};
#undef ATTR
#define ATTR(module, attr)

struct cmd_array {
	enum module_list module;
	int           in;
	int           out;
};

struct module_array {
	int (*func)(int cmd, int in_attrs[], void *in_param[], int out_attrs[], void *out_param[]);
};


extern struct module_array module_array[];
extern struct cmd_array cmd_array[];
extern char *cmd_name[];
extern char *attr_name[];



#undef MODULE
#define MODULE(module, func) extern int f_ ## func (int cmd, int in_attrs[], void *in_param[],  int out_attrs[], void *out_param[]);
#include <sunlla_all.h>
#undef MODULE
#define MODULE(module, func)

#endif /* __SUNLLA_UNIFY_H__ */
