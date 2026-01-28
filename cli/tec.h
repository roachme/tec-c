#ifndef LIBTEC_TEC_H
#define LIBTEC_TEC_H

#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>

#include "aux/hook.h"
#include "aux/color.h"
#include "../lib/libtec.h"

#define PROGRAM     "tec"

// TODO: Get rid of it (set options with default boolean values)
#define NONEBOOL        -1      /* Not yet set boolean value */

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define xstr(s) str(s)
#define str(s) #s

#define IDSIZ           8
#define DESKSIZ         10
#define ENVSIZ          10
#define CMDSIZ          10
#define COLSIZ          7

#define FIRST_COLUMN     0
#define LAST_COLUMN      100

#define xstr(s)     str(s)
#define str(s)      #s

#define IDLIMIT     9999999
#define IDFMT       "%0" xstr(IDSIZ) "u"
#define PADDING_UNIT     6

#define LIST_OBJ_UNITS(_mark, _obj, _pgnout, _desc) do {\
    color_print_str("%-" xstr(COLSIZ) "s ", (_mark), YEL); \
    color_print_str("%-" xstr(IDSIZ) "s ", (_obj), BBLU); \
    color_print_str("%s ", (_pgnout), WHT); \
    color_print_str("%s\n", (_desc), WHT); \
} while (0)

#define CTX_INIT { .column = NULL, .units = NULL, .list = NULL }

enum tec_setup_level {
    TEC_PAGER,
    TEC_SETUP_SOFT,
    TEC_SETUP_HARD,
};

typedef struct builtin {
    const char *name;
    int (*func)(int argc, char **argv, tec_ctx_t * ctx);
    unsigned int option;
} builtin_t;

typedef struct column {
    int prio;
    char *mark;
    char *name;
} column_t;

extern char *unitkeys[];
extern unsigned int nunitkey;

extern column_t builtin_columns[];
extern unsigned int nbuiltin_column;

int is_valid_length(const char *obj, int len);
int check_arg_env(tec_arg_t * args, const char *errfmt, int quiet);
int check_arg_desk(tec_arg_t * args, const char *errfmt, int quiet);
int check_arg_task(tec_arg_t * args, const char *errfmt, int quiet);

bool column_exist(const char *colname);
tec_unit_t *generate_column(char *colname);

int help_list_commands(void);
int help_usage(const char *cmd);
int help_lookup(const char *cmd);

int tec_pwd_unset(void);
int tec_pwd_task(tec_arg_t * args);
int tec_pwd_desk(tec_arg_t * args);
int tec_pwd_env(tec_arg_t * args);
int elog(int status, const char *fmt, ...);
int dlog(int level, const char *fmt, ...);
int llog(int status, const char *fmt, ...);

// NOTE: maybe use 'prefix' like in git?
// int cmd_add(int argc, const char **argv, const char *prefix, struct repository *repo);

// TODO: make argv const
int tec_cli_add(int argc, char **argv, tec_ctx_t * ctx);
int tec_cli_cat(int argc, char **argv, tec_ctx_t * ctx);
int tec_cli_cd(int argc, char **argv, tec_ctx_t * ctx);
int tec_cli_cfg(int argc, char **argv, tec_ctx_t * ctx);
int tec_cli_column(int argc, char **argv, tec_ctx_t * ctx);
int tec_cli_env(int argc, char **argv, tec_ctx_t * ctx);
int tec_cli_desk(int argc, char **argv, tec_ctx_t * ctx);
int tec_cli_help(int argc, char **argv, tec_ctx_t * ctx);
int tec_cli_init(int argc, char **argv, tec_ctx_t * ctx);
int tec_cli_ls(int argc, char **argv, tec_ctx_t * ctx);
int tec_cli_mv(int argc, char **argv, tec_ctx_t * ctx);
int tec_cli_plugin(int argc, char **argv, tec_ctx_t * ctx);
int tec_cli_rm(int argc, char **argv, tec_ctx_t * ctx);
int tec_cli_set(int argc, char **argv, tec_ctx_t * ctx);

#endif
