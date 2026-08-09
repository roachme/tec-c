#include <stdio.h>
#include <dirent.h>
#include <ctype.h>

#include "dir.h"
#include "unit.h"
#include "list.h"
#include "path.h"
#include "osdep.h"
#include "errmod.h"
#include "libtec.h"

struct tecstruct {
    char db[PATH_MAX + 1];      /* directory for tec metadata */
    char base[PATH_MAX + 1];    /* directory for all stuff above */
};

/**
 * is_valid_object() - check whether a name is a legal env/desk/task ID
 * @name: NUL-terminated name to validate
 *
 * A valid name starts and ends with an alphanumeric character, and
 * every character in between is alphanumeric, '_' or '-'.
 *
 * Return: true if @name is valid, false otherwise.
 */
static bool is_valid_object(const char *name)
{
    if (!isalnum(*name++))
        return false;
    for (; *name; ++name)
        if (!(isalnum(*name) || *name == '_' || *name == '-'))
            return false;
    return isalnum(*--name) != false;
}

/**
 * aux_unit_set() - merge new key/val pairs into an object's units file
 * @newunits: linked list of key/val pairs to apply
 * @fname: path of the units file to load, update and save
 *
 * Loads the existing units from @fname, applies each pair in
 * @newunits on top (adding or overwriting keys), saves the result
 * back to @fname, then frees the loaded (merged) list. @newunits
 * itself is not freed or modified.
 *
 * Return: ETEC_OK on success, ETEC_UNIT_LOAD if @fname could not be
 * loaded, ETEC_UNIT_SAVE if the merged units could not be saved.
 */
static int aux_unit_set(tec_unit_t *newunits, const char *fname)
{
    struct tec_unit *item;
    struct tec_unit *units;

    if ((units = unit_load(fname)) == NULL)
        return emod_set(ETEC_UNIT_LOAD);

    // TODO: check values in atomic way

    for (item = newunits; item; item = item->next)
        unit_set(units, item->key, item->val);
    if (unit_save(fname, units))
        return emod_set(ETEC_UNIT_SAVE);

    unit_free(units);
    return ETEC_OK;
}

/**
 * aux_list_get() - list the valid subdirectory entries of a directory
 * @ctx: context whose @ctx->list is populated with one entry per
 *       subdirectory found (appended to any list already present)
 * @dirname: directory to scan
 *
 * Skips dotfiles and non-directory entries. Each remaining entry is
 * added to @ctx->list with status ETEC_OK, or ETEC_ARG_TASK_ILLEG if
 * its name fails is_valid_object().
 *
 * Return: ETEC_OK on success (including when @ctx->list could not be
 * grown for an entry, which is currently not surfaced as an error),
 * or ETEC_DIR_OPEN if @dirname could not be opened.
 */
static int aux_list_get(tec_ctx_t *ctx, const char *dirname)
{
    DIR *ids;
    tec_arg_t args;
    struct dirent *ent;

    if ((ids = opendir(dirname)) == NULL)
        return emod_set(ETEC_DIR_OPEN);

    while ((ent = readdir(ids)) != NULL) {
        int status = ETEC_OK;
        args.task = ent->d_name;

        if (ent->d_name[0] == '.' || ent->d_type != DT_DIR)
            continue;
        else if (is_valid_object(args.task) == false)
            status = ETEC_ARG_TASK_ILLEG;

        ctx->list = list_add(ctx->list, args.task, status);
    }

    closedir(ids);
    return ETEC_OK;
}

/**
 * fill_sysvars() - fill in the well-known tec filesystem paths
 * @taskdir: root directory of the task database
 * @tecfs: struct to fill in with @taskdir and its .tec metadata path
 *
 * Return: ETEC_OK (always succeeds).
 */
static int fill_sysvars(const char *taskdir, struct tecstruct *tecfs)
{
    sprintf(tecfs->base, "%s", taskdir);
    sprintf(tecfs->db, "%s/%s", taskdir, ".tec");
    return ETEC_OK;
}

/**
 * tec_make_db() - initialize a new tec task database
 * @taskdir: root directory in which to create the database
 *
 * Creates @taskdir itself (if missing) and its top-level .tec
 * metadata directory.
 *
 * Return: ETEC_OK on success, ETEC_SYS_MALLOC if the internal paths
 * could not be built, ETEC_DIR_MAKE if @taskdir or its .tec directory
 * could not be created.
 */
int tec_make_db(const char *taskdir)
{
    struct tecstruct tecfs;

    if (fill_sysvars(taskdir, &tecfs))
        return emod_set(ETEC_SYS_MALLOC);
    else if (MKDIR(tecfs.base))
        return emod_set(ETEC_DIR_MAKE);
    else if (MKDIR(tecfs.db))
        return emod_set(ETEC_DIR_MAKE);
    return ETEC_OK;
}

/**
 * tec_check_db() - verify a directory holds an initialized tec database
 * @taskdir: root directory to check
 *
 * Return: ETEC_OK if @taskdir's .tec metadata directory exists,
 * ETEC_SYS_MALLOC if the internal paths could not be built,
 * ETEC_SYS_DB if the .tec metadata directory does not exist.
 */
int tec_check_db(const char *taskdir)
{
    struct tecstruct tecfs;

    if (fill_sysvars(taskdir, &tecfs))
        return emod_set(ETEC_SYS_MALLOC);
    else if (!ISDIR(tecfs.db))
        return emod_set(ETEC_SYS_DB);
    return ETEC_OK;
}

/* TODO: if units fail to create then directory will be leaving.
 * To do it in atomic way: create everything in the tmp directory, and once it
 * is done rename directory.  */
/**
 * tec_task_add() - create a new task and save its initial units
 * @taskdir: root directory of the task database
 * @args: identifiers of the task to create
 * @ctx: @ctx->units holds the initial key/val pairs to save for the task
 *
 * Return: ETEC_OK on success, ETEC_DIR_MAKE if the task directory
 * could not be created, ETEC_UNIT_SAVE if @ctx->units could not be
 * saved.
 */
int tec_task_add(const char *taskdir, tec_arg_t *args, tec_ctx_t *ctx)
{
    if (dir_task_add(taskdir, args) != 0)
        return emod_set(ETEC_DIR_MAKE);
    else if (unit_save(path_task_unit(taskdir, args), ctx->units))
        return emod_set(ETEC_UNIT_SAVE);
    return ETEC_OK;
}

/**
 * tec_task_exist() - check whether a task's directory exists
 * @taskdir: root directory of the task database
 * @args: identifiers of the task to check
 *
 * Return: ETEC_OK if the task directory exists, ETEC_ARG_TASK_NOSUCH
 * otherwise.
 */
int tec_task_exist(const char *taskdir, tec_arg_t *args)
{
    char *pathname = path_task_dir(taskdir, args);
    return ISDIR(pathname) == true ? ETEC_OK : ETEC_ARG_TASK_NOSUCH;
}

/**
 * tec_task_rm() - remove a task and its directory
 * @taskdir: root directory of the task database
 * @args: identifiers of the task to remove
 * @ctx: unused
 *
 * Return: ETEC_OK on success, ETEC_DIR_RM if the task directory could
 * not be removed.
 */
int tec_task_rm(const char *taskdir, tec_arg_t *args, tec_ctx_t *ctx)
{
    (void)args;
    (void)ctx;

    if (dir_task_rm(taskdir, args))
        return emod_set(ETEC_DIR_RM);
    return ETEC_OK;
}

/**
 * tec_task_get() - load a task's units into the context
 * @taskdir: root directory of the task database
 * @args: identifiers of the task to load
 * @ctx: @ctx->units is set to the loaded unit list on success
 *
 * Return: ETEC_OK on success, ETEC_UNIT_GET if the task's units file
 * could not be loaded.
 */
int tec_task_get(const char *taskdir, tec_arg_t *args, tec_ctx_t *ctx)
{
    if ((ctx->units = unit_load(path_task_unit(taskdir, args))) == NULL)
        return emod_set(ETEC_UNIT_GET);
    return ETEC_OK;
}

/**
 * tec_task_list() - list the tasks belonging to a desk
 * @taskdir: root directory of the task database
 * @args: identifiers of the desk whose tasks are listed
 * @ctx: @ctx->list is populated with one entry per task
 *
 * Return: ETEC_OK on success, ETEC_DIR_OPEN if the desk's directory
 * could not be opened.
 */
int tec_task_list(const char *taskdir, tec_arg_t *args, tec_ctx_t *ctx)
{
    return aux_list_get(ctx, path_desk_dir(taskdir, args));
}

/**
 * tec_task_move() - move a task to a different env/desk
 * @taskdir: root directory of the task database
 * @src: identifiers of the task's current location
 * @dst: identifiers of the task's new location
 * @ctx: unused
 *
 * Return: ETEC_OK on success, ETEC_DIR_MOVE if the task directory
 * could not be moved.
 */
int tec_task_move(const char *taskdir, tec_arg_t *src, tec_arg_t *dst,
                  tec_ctx_t *ctx)
{
    (void)ctx;

    if (dir_task_move(taskdir, src, dst))
        return emod_set(ETEC_DIR_MOVE);
    return ETEC_OK;
}

/**
 * tec_task_set() - merge new key/val pairs into a task's units
 * @taskdir: root directory of the task database
 * @args: identifiers of the task to update
 * @ctx: @ctx->units holds the key/val pairs to merge in
 *
 * Return: ETEC_OK on success, ETEC_UNIT_LOAD if the task's existing
 * units could not be loaded, ETEC_UNIT_SAVE if the merged units could
 * not be saved.
 */
int tec_task_set(const char *taskdir, tec_arg_t *args, tec_ctx_t *ctx)
{
    return aux_unit_set(ctx->units, path_task_unit(taskdir, args));
}

/**
 * tec_task_valid() - check whether a task ID is syntactically legal
 * @taskdir: unused
 * @args: identifiers whose @args->task is validated
 *
 * Return: ETEC_OK if @args->task is a legal ID, ETEC_ARG_TASK_ILLEG
 * otherwise.
 */
int tec_task_valid(const char *taskdir, tec_arg_t *args)
{
    (void)taskdir;

    if (is_valid_object(args->task) == false)
        return emod_set(ETEC_ARG_TASK_ILLEG);
    return ETEC_OK;
}

/**
 * tec_desk_add() - create a new desk and save its initial units
 * @taskdir: root directory of the task database
 * @args: identifiers of the desk to create
 * @ctx: @ctx->units holds the initial key/val pairs to save for the desk
 *
 * Return: ETEC_OK on success, ETEC_DIR_MAKE if the desk directory
 * could not be created, ETEC_UNIT_SAVE if @ctx->units could not be
 * saved.
 */
int tec_desk_add(const char *taskdir, tec_arg_t *args, tec_ctx_t *ctx)
{
    if (dir_desk_add(taskdir, args))
        return emod_set(ETEC_DIR_MAKE);
    else if (unit_save(path_desk_unit(taskdir, args), ctx->units))
        return emod_set(ETEC_UNIT_SAVE);
    return ETEC_OK;
}

/**
 * tec_desk_exist() - check whether a desk's directory exists
 * @taskdir: root directory of the task database
 * @args: identifiers of the desk to check
 *
 * Return: ETEC_OK if the desk directory exists, ETEC_ARG_DESK_NOSUCH
 * otherwise.
 */
int tec_desk_exist(const char *taskdir, tec_arg_t *args)
{
    char *pathname = path_desk_dir(taskdir, args);
    return ISDIR(pathname) == true ? ETEC_OK : ETEC_ARG_DESK_NOSUCH;
}

/**
 * tec_desk_rm() - remove a desk and its directory
 * @taskdir: root directory of the task database
 * @args: identifiers of the desk to remove
 * @ctx: unused
 *
 * Return: ETEC_OK on success, ETEC_DIR_RM if the desk directory could
 * not be removed.
 */
int tec_desk_rm(const char *taskdir, tec_arg_t *args, tec_ctx_t *ctx)
{
    (void)ctx;

    if (dir_desk_rm(taskdir, args))
        return emod_set(ETEC_DIR_RM);
    return ETEC_OK;
}

/**
 * tec_desk_get() - load a desk's units into the context
 * @taskdir: root directory of the task database
 * @args: identifiers of the desk to load
 * @ctx: @ctx->units is set to the loaded unit list on success
 *
 * Return: ETEC_OK on success, ETEC_UNIT_GET if the desk's units file
 * could not be loaded.
 */
int tec_desk_get(const char *taskdir, tec_arg_t *args, tec_ctx_t *ctx)
{
    if ((ctx->units = unit_load(path_desk_unit(taskdir, args))) == NULL)
        return emod_set(ETEC_UNIT_GET);
    return ETEC_OK;
}

/**
 * tec_desk_list() - list the desks belonging to an environment
 * @taskdir: root directory of the task database
 * @args: identifiers of the environment whose desks are listed
 * @ctx: @ctx->list is populated with one entry per desk
 *
 * Return: ETEC_OK on success, ETEC_DIR_OPEN if the environment's
 * directory could not be opened.
 */
int tec_desk_list(const char *taskdir, tec_arg_t *args, tec_ctx_t *ctx)
{
    return aux_list_get(ctx, path_env_dir(taskdir, args));
}

/**
 * tec_desk_move() - move a desk to a different environment
 * @taskdir: root directory of the task database
 * @src: identifiers of the desk's current location
 * @dst: identifiers of the desk's new location
 * @ctx: unused
 *
 * Return: ETEC_OK on success, ETEC_DIR_MOVE if the desk directory
 * could not be moved.
 */
int tec_desk_move(const char *taskdir, tec_arg_t *src, tec_arg_t *dst,
                  tec_ctx_t *ctx)
{
    (void)ctx;

    if (dir_desk_move(taskdir, src, dst))
        return emod_set(ETEC_DIR_MOVE);
    return ETEC_OK;
}

/**
 * tec_desk_set() - merge new key/val pairs into a desk's units
 * @taskdir: root directory of the task database
 * @args: identifiers of the desk to update
 * @ctx: @ctx->units holds the key/val pairs to merge in
 *
 * Return: ETEC_OK on success, ETEC_UNIT_LOAD if the desk's existing
 * units could not be loaded, ETEC_UNIT_SAVE if the merged units could
 * not be saved.
 */
int tec_desk_set(const char *taskdir, tec_arg_t *args, tec_ctx_t *ctx)
{
    (void)ctx;
    return aux_unit_set(ctx->units, path_desk_unit(taskdir, args));
}

/**
 * tec_desk_valid() - check whether a desk name is syntactically legal
 * @taskdir: unused
 * @args: identifiers whose @args->desk is validated
 *
 * Return: ETEC_OK if @args->desk is a legal name, ETEC_ARG_DESK_ILLEG
 * otherwise.
 */
int tec_desk_valid(const char *taskdir, tec_arg_t *args)
{
    (void)taskdir;

    if (is_valid_object(args->desk) == false)
        return emod_set(ETEC_ARG_DESK_ILLEG);
    return ETEC_OK;
}

/**
 * tec_env_add() - create a new environment and save its initial units
 * @taskdir: root directory of the task database
 * @args: identifiers of the environment to create
 * @ctx: @ctx->units holds the initial key/val pairs to save for the
 *       environment
 *
 * Return: ETEC_OK on success, ETEC_DIR_MAKE if the environment
 * directory could not be created, ETEC_UNIT_SAVE if @ctx->units could
 * not be saved.
 */
int tec_env_add(const char *taskdir, tec_arg_t *args, tec_ctx_t *ctx)
{
    if (dir_env_add(taskdir, args))
        return emod_set(ETEC_DIR_MAKE);
    else if (unit_save(path_env_unit(taskdir, args), ctx->units))
        return emod_set(ETEC_UNIT_SAVE);
    return ETEC_OK;
}

/**
 * tec_env_exist() - check whether an environment's directory exists
 * @taskdir: root directory of the task database
 * @args: identifiers of the environment to check
 *
 * Return: ETEC_OK if the environment directory exists,
 * ETEC_ARG_ENV_NOSUCH otherwise.
 */
int tec_env_exist(const char *taskdir, tec_arg_t *args)
{
    char *pathname = path_env_dir(taskdir, args);
    return ISDIR(pathname) == true ? ETEC_OK : ETEC_ARG_ENV_NOSUCH;
}

/**
 * tec_env_rm() - remove an environment and its directory
 * @taskdir: root directory of the task database
 * @args: identifiers of the environment to remove
 * @ctx: unused
 *
 * Return: ETEC_OK on success, ETEC_DIR_RM if the environment
 * directory could not be removed.
 */
int tec_env_rm(const char *taskdir, tec_arg_t *args, tec_ctx_t *ctx)
{
    (void)ctx;

    if (dir_env_rm(taskdir, args))
        return emod_set(ETEC_DIR_RM);
    return ETEC_OK;
}

/**
 * tec_env_get() - load an environment's units into the context
 * @taskdir: root directory of the task database
 * @args: identifiers of the environment to load
 * @ctx: @ctx->units is set to the loaded unit list on success
 *
 * Return: ETEC_OK on success, ETEC_UNIT_LOAD if the environment's
 * units file could not be loaded.
 */
int tec_env_get(const char *taskdir, tec_arg_t *args, tec_ctx_t *ctx)
{
    if ((ctx->units = unit_load(path_env_unit(taskdir, args))) == NULL)
        return emod_set(ETEC_UNIT_LOAD);
    return ETEC_OK;
}

/**
 * tec_env_list() - list the environments in the task database
 * @taskdir: root directory of the task database
 * @args: unused
 * @ctx: @ctx->list is populated with one entry per environment
 *
 * Return: ETEC_OK on success, ETEC_DIR_OPEN if @taskdir could not be
 * opened.
 */
int tec_env_list(const char *taskdir, tec_arg_t *args, tec_ctx_t *ctx)
{
    (void)args;
    return aux_list_get(ctx, taskdir);
}

/**
 * tec_env_rename() - rename an environment
 * @taskdir: root directory of the task database
 * @src: identifiers holding the environment's current name
 * @dst: identifiers holding the environment's new name
 * @ctx: unused
 *
 * Return: ETEC_OK on success, ETEC_DIR_RENAME if the environment
 * directory could not be renamed.
 */
int tec_env_rename(const char *taskdir, tec_arg_t *src, tec_arg_t *dst,
                   tec_ctx_t *ctx)
{
    (void)ctx;

    if (dir_env_rename(taskdir, src, dst))
        return emod_set(ETEC_DIR_RENAME);
    return ETEC_OK;
}

/**
 * tec_env_set() - merge new key/val pairs into an environment's units
 * @taskdir: root directory of the task database
 * @args: identifiers of the environment to update
 * @ctx: @ctx->units holds the key/val pairs to merge in
 *
 * Return: ETEC_OK on success, ETEC_UNIT_LOAD if the environment's
 * existing units could not be loaded, ETEC_UNIT_SAVE if the merged
 * units could not be saved.
 */
int tec_env_set(const char *taskdir, tec_arg_t *args, tec_ctx_t *ctx)
{
    return aux_unit_set(ctx->units, path_env_unit(taskdir, args));
}

/**
 * tec_env_valid() - check whether an environment name is syntactically legal
 * @taskdir: unused
 * @args: identifiers whose @args->env is validated
 *
 * Return: ETEC_OK if @args->env is a legal name, ETEC_ARG_ENV_ILLEG
 * otherwise.
 */
int tec_env_valid(const char *taskdir, tec_arg_t *args)
{
    (void)taskdir;

    if (is_valid_object(args->env) == false)
        return emod_set(ETEC_ARG_ENV_ILLEG);
    return ETEC_OK;
}

/**
 * tec_list_free() - free a listing array returned by a tec_*_list() call
 * @list: listing to free, may be NULL
 *
 * Return: NULL always, for convenient use as `list = tec_list_free(list);`.
 */
void *tec_list_free(tec_listarr_t *list)
{
    list_free(list);
    return NULL;
}

/**
 * tec_geterr() - get the human-readable message for an error code
 * @errnum: an ETEC_* error code (or ETEC_OK) to translate
 *
 * Return: a pointer to a static buffer holding the message for
 * @errnum; see emod_geterr() for details.
 */
char *tec_geterr(int errnum)
{
    return emod_geterr(errnum);
}

/**
 * tec_unit_add() - append a new key/val node to the end of a unit list
 * @head: head of the list to append to, may be NULL
 * @key: key string, copied internally
 * @val: value string, copied internally
 *
 * Return: @head with the new node appended, or the new node itself if
 * @head was NULL; see unit_add() for details.
 */
tec_unit_t *tec_unit_add(struct tec_unit *head, char *key, char *val)
{
    return unit_add(head, key, val);
}

/**
 * tec_unit_join() - join two unit lists into one without copying data
 * @head: head of the first list, may be NULL
 * @body: list to attach after @head's last node
 *
 * Return: @head with @body linked onto its end, or @body if @head was
 * NULL.
 */
tec_unit_t *tec_unit_join(tec_unit_t *head, tec_unit_t *body)
{
    return unit_join(head, body);
}

/**
 * tec_unit_set() - update the value for a key, or add it if not present
 * @head: head of the unit list to update
 * @key: key to look up
 * @val: new value string, copied internally
 *
 * Return: ETEC_OK always (unit_set() never reports failure through its
 * return value, it silently leaves the list unchanged on allocation
 * failure).
 */
int tec_unit_set(struct tec_unit *head, char *key, char *val)
{
    // TODO: this will never fail, cuz in case of error it returns head.
    return !unit_set(head, key, val) ? ETEC_OK : ETEC_UNIT_SET;
}

/**
 * tec_unit_save() - write a unit list to a file
 * @filename: path of the file to write
 * @units: head of the linked list of units to write, may be NULL
 *
 * Return: 0 on success, nonzero on failure; see unit_save() for the
 * failure conditions.
 */
int tec_unit_save(const char *filename, tec_unit_t *units)
{
    return unit_save(filename, units);
}

/**
 * tec_unit_parse() - parse a single "key : val" line and add it to a list
 * @head: head of the list to append the parsed unit to, may be NULL
 * @str: line of text to parse, in "key : val" form
 *
 * Return: @head with the new key/val node appended if parsing
 * succeeded, otherwise @head unchanged.
 */
tec_unit_t *tec_unit_parse(struct tec_unit *head, const char *str)
{
    return unit_parse(head, str);
}

/**
 * tec_unit_get() - look up the value stored for a key
 * @head: head of the unit list to search, may be NULL
 * @key: key to look up
 *
 * Return: a pointer to the matching node's value (not a copy), or
 * NULL if no node in @head has a matching key.
 */
char *tec_unit_get(tec_unit_t *head, char *key)
{
    return unit_get(head, key);
}

/**
 * tec_unit_free() - free every node in a unit list
 * @units: head of the unit list to free, may be NULL
 *
 * Return: NULL always, for convenient use as `units = tec_unit_free(units);`.
 */
void *tec_unit_free(tec_unit_t *units)
{
    unit_free(units);
    return NULL;
}
