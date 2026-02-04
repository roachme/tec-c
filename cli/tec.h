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

typedef struct argvec {
    char **argv;
    int count;
    int capac;
} tec_argvec_t;

typedef struct builtin {
    const char *name;
    int (*func)(int argc, const char **argv, tec_ctx_t * ctx);
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

void argvec_init(tec_argvec_t * vec);
void argvec_add(tec_argvec_t * vec, const char *arg);
void argvec_parse(tec_argvec_t * vec, int argc, const char **argv);
void argvec_replace(tec_argvec_t * vec, int vec_idx, char *arg, int argsiz);
void argvec_free(tec_argvec_t * vec);

int is_valid_length(const char *obj, int len);
int check_arg_env(tec_arg_t * args, const char *errfmt, int quiet);
int check_arg_desk(tec_arg_t * args, const char *errfmt, int quiet);
int check_arg_task(tec_arg_t * args, const char *errfmt, int quiet);

bool column_exist(const char *colname);
tec_unit_t *generate_column(char *colname);

int help_list_pretty_commands(void);
int help_usage(const char *cmd);
int help_lookup(const char *cmd);

bool tec_cli_get_user_choice(void);

int tec_pwd_unset(void);
int tec_pwd_task(tec_arg_t * args);
int tec_pwd_desk(tec_arg_t * args);
int tec_pwd_env(tec_arg_t * args);
int elog(int status, const char *fmt, ...);
int dlog(int level, const char *fmt, ...);
int llog(int status, const char *fmt, ...);

// NOTE: maybe use 'prefix' like in git?
// int cmd_add(int argc, const char **argv, const char *prefix, struct repository *repo);

int tec_cli_add(int argc, const char **argv, tec_ctx_t * ctx);
int tec_cli_cat(int argc, const char **argv, tec_ctx_t * ctx);
int tec_cli_cd(int argc, const char **argv, tec_ctx_t * ctx);
int tec_cli_cfg(int argc, const char **argv, tec_ctx_t * ctx);
int tec_cli_column(int argc, const char **argv, tec_ctx_t * ctx);
int tec_cli_env(int argc, const char **argv, tec_ctx_t * ctx);
int tec_cli_desk(int argc, const char **argv, tec_ctx_t * ctx);
int tec_cli_help(int argc, const char **argv, tec_ctx_t * ctx);
int tec_cli_init(int argc, const char **argv, tec_ctx_t * ctx);
int tec_cli_ls(int argc, const char **argv, tec_ctx_t * ctx);
int tec_cli_mv(int argc, const char **argv, tec_ctx_t * ctx);
int tec_cli_plugin(int argc, const char **argv, tec_ctx_t * ctx);
int tec_cli_rm(int argc, const char **argv, tec_ctx_t * ctx);
int tec_cli_set(int argc, const char **argv, tec_ctx_t * ctx);

#endif
