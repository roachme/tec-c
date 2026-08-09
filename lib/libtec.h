#ifndef TEC_H
#define TEC_H

#include <stddef.h>

/**
 * struct tec_arg - task/desk/env identifiers threaded through lib and cli
 * @task: task ID, or NULL when unset/unused by the caller
 * @desk: desk name, or NULL when unset/unused by the caller
 * @env: environment name, or NULL when unset/unused by the caller
 */
typedef struct tec_arg {
    char *task;
    char *desk;
    char *env;
} tec_arg_t;

/**
 * struct tec_unit - one key/val pair in a unit key-value store
 * @key: the unit's key string
 * @val: the unit's value string
 * @next: next node in the singly linked list, or NULL at the tail
 */
typedef struct tec_unit {
    char *key;
    char *val;
    struct tec_unit *next;
} tec_unit_t;

/**
 * struct tec_list - one entry in a tec_listarr_t listing
 * @name: name of the listed env/desk/task
 * @status: status code of the object (an ETEC_* value, ETEC_OK if valid)
 */
typedef struct tec_list {
    char *name;
    unsigned char status;       /* Status code of the object */
} tec_list_t;

/**
 * struct tec_listarr - dynamic array of tec_list_t entries
 * @items: backing array of entries
 * @count: number of entries currently stored in @items
 * @cap: allocated capacity of @items, in entries
 */
typedef struct tec_listarr {
    tec_list_t *items;
    size_t count;
    size_t cap;
} tec_listarr_t;

/**
 * struct tec_context - out-parameter carrying results of tec_* calls
 * @units: parsed unit list filled in by unit-returning calls, or NULL
 * @list: listing filled in by tec_*_list() calls, or NULL
 */
typedef struct tec_context {
    struct tec_unit *units;
    struct tec_listarr *list;
} tec_ctx_t;

/**
 * enum tec_errno - error codes returned by lib/ functions
 * @ETEC_OK: success
 * @ETEC_SYS_DB: database directory not found
 * @ETEC_SYS_MALLOC: memory allocation failed
 * @ETEC_ARG_TASK_ILLEG: task ID contains illegal characters
 * @ETEC_ARG_DESK_ILLEG: desk name contains illegal characters
 * @ETEC_ARG_ENV_ILLEG: environment name contains illegal characters
 * @ETEC_ARG_TASK_NOSUCH: no task with the given ID exists
 * @ETEC_ARG_DESK_NOSUCH: no desk with the given name exists
 * @ETEC_ARG_ENV_NOSUCH: no environment with the given name exists
 * @ETEC_ARG_TASK_EXIST: a task with the given ID already exists
 * @ETEC_ARG_DESK_EXIST: a desk with the given name already exists
 * @ETEC_ARG_ENV_EXIST: an environment with the given name already exists
 * @ETEC_DIR_RM: a directory could not be removed
 * @ETEC_DIR_MAKE: a directory could not be created
 * @ETEC_DIR_MOVE: a directory could not be moved
 * @ETEC_DIR_OPEN: a directory could not be opened
 * @ETEC_DIR_RENAME: a directory could not be renamed
 * @ETEC_UNIT_ADD: a unit node could not be added
 * @ETEC_UNIT_RM: a unit node could not be removed
 * @ETEC_UNIT_GET: unit values could not be retrieved
 * @ETEC_UNIT_ILLEG: a unit value is illegal
 * @ETEC_UNIT_KEY: the requested unit key does not exist
 * @ETEC_UNIT_LOAD: units could not be loaded from disk
 * @ETEC_UNIT_SAVE: unit values could not be saved to disk
 * @ETEC_UNIT_SET: unit values could not be set
 * @__ETEC_STATUS_LAST: sentinel marking the end of the enum, not a
 *                      real error code
 */
enum tec_errno {
    ETEC_OK,

    ETEC_SYS_DB,
    ETEC_SYS_MALLOC,

    ETEC_ARG_TASK_ILLEG,
    ETEC_ARG_DESK_ILLEG,
    ETEC_ARG_ENV_ILLEG,

    ETEC_ARG_TASK_NOSUCH,
    ETEC_ARG_DESK_NOSUCH,
    ETEC_ARG_ENV_NOSUCH,

    ETEC_ARG_TASK_EXIST,
    ETEC_ARG_DESK_EXIST,
    ETEC_ARG_ENV_EXIST,

    ETEC_DIR_RM,
    ETEC_DIR_MAKE,
    ETEC_DIR_MOVE,
    ETEC_DIR_OPEN,
    ETEC_DIR_RENAME,

    ETEC_UNIT_ADD,
    ETEC_UNIT_RM,
    ETEC_UNIT_GET,
    ETEC_UNIT_ILLEG,
    ETEC_UNIT_KEY,
    ETEC_UNIT_LOAD,
    ETEC_UNIT_SAVE,
    ETEC_UNIT_SET,

    __ETEC_STATUS_LAST
};

/* Core functions.  */
int tec_make_db(const char *taskdir);
int tec_check_db(const char *taskdir);
char *tec_geterr(int errnum);

/* Data structure for unit values.  */
tec_unit_t *tec_unit_add(tec_unit_t * head, char *key, char *val);
tec_unit_t *tec_unit_join(tec_unit_t * head, tec_unit_t * body);
tec_unit_t *tec_unit_parse(struct tec_unit *head, const char *str);
char *tec_unit_get(tec_unit_t * head, char *key);
void *tec_unit_free(tec_unit_t * units);
int tec_unit_set(struct tec_unit *head, char *key, char *val);
int tec_unit_save(const char *filename, tec_unit_t * units);

/* Data structure for list of objects.  */
void *tec_list_free(tec_listarr_t * list);

/* Task functions.  */
int tec_task_add(const char *taskdir, tec_arg_t * args, tec_ctx_t * ctx);
int tec_task_exist(const char *taskdir, tec_arg_t * args);
int tec_task_get(const char *taskdir, tec_arg_t * args, tec_ctx_t * ctx);
int tec_task_valid(const char *taskdir, tec_arg_t * args);
int tec_task_list(const char *taskdir, tec_arg_t * args, tec_ctx_t * ctx);
int tec_task_move(const char *taskdir, tec_arg_t * src, tec_arg_t * dst,
                  tec_ctx_t * ctx);
int tec_task_rm(const char *taskdir, tec_arg_t * args, tec_ctx_t * ctx);
int tec_task_set(const char *taskdir, tec_arg_t * args, tec_ctx_t * ctx);

/* Desk functions.  */
int tec_desk_add(const char *taskdir, tec_arg_t * args, tec_ctx_t * ctx);
int tec_desk_exist(const char *taskdir, tec_arg_t * args);
int tec_desk_get(const char *taskdir, tec_arg_t * args, tec_ctx_t * ctx);
int tec_desk_valid(const char *taskdir, tec_arg_t * args);
int tec_desk_list(const char *taskdir, tec_arg_t * args, tec_ctx_t * ctx);
int tec_desk_move(const char *taskdir, tec_arg_t * src, tec_arg_t * dst,
                  tec_ctx_t * ctx);
int tec_desk_rm(const char *taskdir, tec_arg_t * args, tec_ctx_t * ctx);
int tec_desk_set(const char *taskdir, tec_arg_t * args, tec_ctx_t * ctx);

/* Environment functions.  */
int tec_env_add(const char *taskdir, tec_arg_t * args, tec_ctx_t * ctx);
int tec_env_exist(const char *taskdir, tec_arg_t * args);
int tec_env_valid(const char *taskdir, tec_arg_t * args);
int tec_env_get(const char *taskdir, tec_arg_t * args, tec_ctx_t * ctx);
int tec_env_list(const char *taskdir, tec_arg_t * args, tec_ctx_t * ctx);
int tec_env_rename(const char *taskdir, tec_arg_t * src, tec_arg_t * dst,
                   tec_ctx_t * ctx);
int tec_env_rm(const char *taskdir, tec_arg_t * args, tec_ctx_t * ctx);
int tec_env_set(const char *taskdir, tec_arg_t * args, tec_ctx_t * ctx);

#endif
