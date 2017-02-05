/* Author	: Vedic.Fu
 * E-mail	: FZKmxcz@163.com
 * Copyright	: Free
 * Description	: parse macro "MODULE", "CMD", "ATTR"
 */

#include "sunlla_unify.h"

/* Module functions */
#undef MODULE
#define MODULE(module, func) { f_##func },
struct module_array module_array[] = {
	#include <sunlla_all.h>
};
#undef MODULE
#define MODULE(module, func)


/* Command parameters */
#undef CMD
#define CMD(module, op, in, out) { MODULE_##module, in, out },
struct cmd_array cmd_array[] = {
	#include <sunlla_all.h>
};
#undef CMD
#define CMD(module, op, in, out)	


/* Command name */
#undef CMD
#define CMD(module, op, in, out) #module"_"#op,
char *cmd_name[] = {
	#include <sunlla_all.h>
};
#undef CMD
#define CMD(module, op, in, out)

/* Field name */
#undef ATTR
#define ATTR(module, attr) #module"_"#attr,
char *attr_name[] = {
	#include <sunlla_all.h>
};
#undef ATTR
#define ATTR(module, attr)

