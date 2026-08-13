#ifndef TEC_CLI_HOOK_H
#define TEC_CLI_HOOK_H

#include "config.h"
#include "../../lib/libtec.h"

/**
 * struct tec_hook - single plugin hook entry from the config file
 * @cmd: tec command name this hook fires on (e.g. "add", "cat")
 * @cmdopt: option passed to the tec command (currently unused by parsing)
 * @pgname: plugin directory/binary name
 * @pgncmd: subcommand passed to the plugin
 * @pgnopt: option passed to the plugin (currently unused by parsing)
 * @next: next hook in the singly linked list
 */
// FIXME: get rid of magic numbers. Might cause buffer overflow
struct tec_hook {
    char cmd[10];
    char cmdopt[10];
    char pgname[10];
    char pgncmd[10];
    char pgnopt[10];
    struct tec_hook *next;
};

int hook_action(tec_arg_t * args, char *cmd, tec_cfg_t * cfg);
int hook_cat(tec_unit_t ** units, tec_arg_t * args, char *cmd, tec_cfg_t * cfg);
/* TODO: under development.  */
char *hook_list(struct tec_hook *hooks, char *pgnout, char *env, char *id);

#endif
