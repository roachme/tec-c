#ifndef CONFIG_H
#define CONFIG_H

#include "../../lib/libtec.h"

#define CONFIGSIZ       256

// FIXME: add an expression max_hookact + max_hookshow + max_hookls
#define CONF_MAXHOOK    10
#define CONF_MAXCOLDEF  10
#define CONF_MAXBASE    256
#define CONF_MAXPGNINS  256

/**
 * struct tec_base - base filesystem locations used by tec
 * @cfg: path to the config file that was loaded (NULL if none)
 * @pgn: directory where plugins are stored
 * @task: directory where tasks are stored
 */
typedef struct tec_base {
    char *cfg;                  /* Path to config filename */
    char *pgn;                  /* Directory where plugins are stored */
    char *task;                 /* Directory where tasks are stored */
} tec_base_t;

/**
 * struct tec_option - global CLI/config boolean options
 * @color: use colors (or NONEBOOL if not yet decided)
 * @debug: output debug info (or NONEBOOL if not yet decided)
 * @hook: execute hooks from config, on by default (or NONEBOOL if not yet decided)
 */
typedef struct tec_option {
    int color;                  /* use colors */
    int debug;                  /* output debug info */
    int hook;                   /* execute hooks from config, by default set */
} tec_opt_t;

/**
 * struct tec_alias - single command alias entry from the config file
 * @name: alias name
 * @cmd: command the alias expands to
 * @next: next alias in the singly linked list
 */
typedef struct tec_alias {
    char name[10];
    char cmd[30];
    struct tec_alias *next;
} tec_alias_t;

/**
 * struct config - top-level, fully parsed tec configuration
 * @opts: global boolean options
 * @base: base filesystem locations
 * @alias: head of the linked list of configured aliases
 * @hooks: head of the linked list of configured hooks
 */
typedef struct config {
    tec_opt_t opts;
    tec_base_t base;
    tec_alias_t *alias;
    struct tec_hook *hooks;
} tec_cfg_t;

#define TEC_CONFIG_SET_OPTS { .opts.color = NONEBOOL, .opts.hook = NONEBOOL, }

int tec_config_parse(tec_cfg_t * tec_config);
void tec_config_destroy(tec_cfg_t * tec_config);

#endif
