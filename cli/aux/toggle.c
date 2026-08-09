#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>

#include "../tec.h"
#include "errno.h"

#define TOGSUFF         ".tec/toggles"
#define TOGENV          "%s/" TOGSUFF
#define TOGDESK         "%s/%s/" TOGSUFF
#define TOGTASK         "%s/%s/%s/" TOGSUFF

static char env_curr[ENVSIZ + 1];
static char env_prev[ENVSIZ + 1];
static char desk_curr[DESKSIZ + 1];
static char desk_prev[DESKSIZ + 1];
static char task_curr[IDSIZ + 1];
static char task_prev[IDSIZ + 1];

/**
 * path_generic() - Format a path into a caller-supplied buffer, checking for truncation
 * @buf: destination buffer
 * @bufsiz: size of @buf
 * @fmt: printf-style path format string
 * @...: arguments for @fmt
 *
 * Return: @buf on success, or NULL if the formatted result would have
 * been truncated (vsnprintf() return value >= @bufsiz)
 */
static char *path_generic(char *buf, int bufsiz, const char *fmt, ...)
{
    int len;
    va_list arg;

    va_start(arg, fmt);
    len = vsnprintf(buf, bufsiz, fmt, arg);
    va_end(arg);

    if (len >= bufsiz)
        return NULL;
    return buf;
}

/**
 * path_env_toggle() - Build the path to an environment's toggle file
 * @base: task base directory
 * @args: unused (kept for signature symmetry with the desk/task variants)
 *
 * Return: pointer to a static buffer holding "<base>/.tec/toggles";
 * overwritten on the next call to this function. NULL if the path
 * would not fit in PATH_MAX.
 */
static char *path_env_toggle(char *base, const tec_arg_t *args)
{
    (void)args;
    const char *fmt = TOGENV;
    static char pathname[PATH_MAX + 1];
    return path_generic(pathname, PATH_MAX, fmt, base);
}

/**
 * path_desk_toggle() - Build the path to a desk's toggle file
 * @base: task base directory
 * @args: env/desk/task selection; only args->env is used
 *
 * Return: pointer to a static buffer holding
 * "<base>/<env>/.tec/toggles"; overwritten on the next call to this
 * function. NULL if the path would not fit in PATH_MAX.
 */
static char *path_desk_toggle(char *base, const tec_arg_t *args)
{
    const char *fmt = TOGDESK;
    static char pathname[PATH_MAX + 1];
    return path_generic(pathname, PATH_MAX, fmt, base, args->env);
}

/**
 * path_task_toggle() - Build the path to a task's toggle file
 * @base: task base directory
 * @args: env/desk/task selection; only args->env and args->desk are used
 *
 * Return: pointer to a static buffer holding
 * "<base>/<env>/<desk>/.tec/toggles"; overwritten on the next call to
 * this function. NULL if the path would not fit in PATH_MAX.
 */
static char *path_task_toggle(char *base, const tec_arg_t *args)
{
    const char *fmt = TOGTASK;
    static char pathname[PATH_MAX + 1];
    return path_generic(pathname, PATH_MAX, fmt, base, args->env, args->desk);
}

/**
 * _get_toggle() - Read a single named value out of a toggle file
 * @fname: path to the toggle file to read
 * @buf: caller-owned buffer to fill with the value; on entry it is
 *       cleared to empty before any read is attempted
 * @bufsiz: capacity of @buf
 * @tog: key to look up in the toggle file (e.g. "curr" or "prev")
 *
 * Opens @fname, parses every line as a tec unit ("key = value"), and
 * copies the value for @tog into @buf if present.
 *
 * Return: @buf if @tog was found and copied into it, NULL if @fname
 * could not be opened or @tog was not present in the file
 */
static char *_get_toggle(const char *fname, char *buf, int bufsiz, char *tog)
{
    FILE *fp;
    char *toggle = NULL;
    tec_unit_t *units = NULL;
    char buffer[BUFSIZ + 1] = { 0 };

    /* Remove previous value if any.  */
    buf[0] = '\0';

    if ((fp = fopen(fname, "r")) == NULL)
        return NULL;

    while (fgets(buffer, BUFSIZ, fp) != NULL)
        units = tec_unit_parse(units, buffer);

    if ((toggle = tec_unit_get(units, tog))) {
        strncpy(buf, toggle, bufsiz);
        buf[bufsiz] = '\0';
    }

    fclose(fp);
    tec_unit_free(units);
    return buf[0] == '\0' ? NULL : buf;
}

/**
 * env_get_curr() - Read the current environment toggle value
 * @base: task base directory
 * @args: env/desk/task selection, passed through to path_env_toggle()
 *
 * Return: pointer to a static buffer holding the current env name, or
 * NULL if unset; overwritten on the next call to this function
 */
static char *env_get_curr(char *base, tec_arg_t *args)
{
    char *path = path_env_toggle(base, args);
    return _get_toggle(path, env_curr, ENVSIZ, "curr");
}

/**
 * env_get_prev() - Read the previous environment toggle value
 * @base: task base directory
 * @args: env/desk/task selection, passed through to path_env_toggle()
 *
 * Return: pointer to a static buffer holding the previous env name,
 * or NULL if unset; overwritten on the next call to this function
 */
static char *env_get_prev(char *base, tec_arg_t *args)
{
    char *path = path_env_toggle(base, args);
    return _get_toggle(path, env_prev, ENVSIZ, "prev");
}

/**
 * desk_get_curr() - Read the current desk toggle value
 * @base: task base directory
 * @args: env/desk/task selection, passed through to path_desk_toggle()
 *
 * Return: pointer to a static buffer holding the current desk name,
 * or NULL if unset; overwritten on the next call to this function
 */
static char *desk_get_curr(char *base, tec_arg_t *args)
{
    char *path = path_desk_toggle(base, args);
    return _get_toggle(path, desk_curr, DESKSIZ, "curr");
}

/**
 * desk_get_prev() - Read the previous desk toggle value
 * @base: task base directory
 * @args: env/desk/task selection, passed through to path_desk_toggle()
 *
 * Return: pointer to a static buffer holding the previous desk name,
 * or NULL if unset; overwritten on the next call to this function
 */
static char *desk_get_prev(char *base, tec_arg_t *args)
{
    char *path = path_desk_toggle(base, args);
    return _get_toggle(path, desk_prev, DESKSIZ, "prev");
}

/**
 * task_get_curr() - Read the current task toggle value
 * @base: task base directory
 * @args: env/desk/task selection, passed through to path_task_toggle()
 *
 * Return: pointer to a static buffer holding the current task ID, or
 * NULL if unset; overwritten on the next call to this function
 */
static char *task_get_curr(char *base, tec_arg_t *args)
{
    char *path = path_task_toggle(base, args);
    return _get_toggle(path, task_curr, IDSIZ, "curr");
}

/**
 * task_get_prev() - Read the previous task toggle value
 * @base: task base directory
 * @args: env/desk/task selection, passed through to path_task_toggle()
 *
 * Return: pointer to a static buffer holding the previous task ID, or
 * NULL if unset; overwritten on the next call to this function
 */
static char *task_get_prev(char *base, tec_arg_t *args)
{
    char *path = path_task_toggle(base, args);
    return _get_toggle(path, task_prev, IDSIZ, "prev");
}

/**
 * toggle_env_get_curr() - Resolve args->env from the current environment toggle
 * @base: task base directory
 * @args: env/desk/task selection; args->env is left untouched if
 *        already set, otherwise filled in from the on-disk toggle
 *
 * Return: ETEC_OK if args->env was already set or was resolved from
 * the toggle, ETEC_TOGG_ENV_GET_CURR if it was unset and no current
 * environment toggle exists
 */
int toggle_env_get_curr(char *base, tec_arg_t *args)
{
    if (!args->env && !(args->env = env_get_curr(base, args)))
        return ETEC_TOGG_ENV_GET_CURR;
    return ETEC_OK;
}

/**
 * toggle_env_get_prev() - Resolve args->env from the previous environment toggle
 * @base: task base directory
 * @args: env/desk/task selection; args->env is left untouched if
 *        already set, otherwise filled in from the on-disk toggle
 *
 * Return: EXIT_SUCCESS if args->env was already set or was resolved
 * from the toggle, EXIT_FAILURE if it was unset and no previous
 * environment toggle exists
 */
int toggle_env_get_prev(char *base, tec_arg_t *args)
{
    if (!args->env && !(args->env = env_get_prev(base, args)))
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

/**
 * toggle_env_is_curr() - Check whether args->env is the current environment
 * @base: task base directory
 * @args: env/desk/task selection; args->env is compared against the
 *        on-disk current toggle
 *
 * Return: true if the current environment toggle equals args->env,
 * false if there is no current toggle or it differs
 */
bool toggle_env_is_curr(char *base, tec_arg_t *args)
{
    char *env;

    /* There is no current env - immediately return false.  */
    if ((env = env_get_curr(base, args)) == NULL)
        return false;
    return !strcmp(env, args->env);
}

/**
 * toggle_env_is_prev() - Check whether args->env is the previous environment
 * @base: task base directory
 * @args: env/desk/task selection; args->env is compared against the
 *        on-disk previous toggle
 *
 * Return: true if the previous environment toggle equals args->env,
 * false if there is no previous toggle or it differs
 */
bool toggle_env_is_prev(char *base, tec_arg_t *args)
{
    char *env;

    /* There is no current env - immediately return false.  */
    if ((env = env_get_prev(base, args)) == NULL)
        return false;
    return !strcmp(env, args->env);
}

/**
 * toggle_env_update() - Rewrite an environment's curr/prev toggle after a rename
 * @base: task base directory
 * @args: env/desk/task selection identifying the toggle file's location
 * @src: old environment name to match against curr/prev
 * @dst: new environment name to replace @src with
 *
 * If the current or previous toggle value equals @src, replaces it
 * with @dst and rewrites the toggle file with both (updated) values.
 * Does nothing if neither curr nor prev matched @src.
 *
 * Return: always 0
 */
int toggle_env_update(char *base, tec_arg_t *args, const char *src,
                      const char *dst)
{
    char *curr, *prev;
    tec_unit_t *toggles;
    int changed = 0;

    toggles = NULL;
    curr = env_get_curr(base, args);
    prev = env_get_prev(base, args);

    /* Check if old_id matches curr or prev and update */
    if (curr && strcmp(curr, src) == 0) {
        curr = (char *)dst;
        changed = 1;
    }
    if (prev && strcmp(prev, src) == 0) {
        prev = (char *)dst;
        changed = 1;
    }

    if (!changed)
        return 0;               /* Nothing to update */

    if (curr)
        toggles = tec_unit_add(toggles, "curr", curr);
    if (prev)
        toggles = tec_unit_add(toggles, "prev", prev);

    if (toggles) {
        tec_unit_save(path_env_toggle(base, args), toggles);
        tec_unit_free(toggles);
    }
    return 0;
}

/**
 * toggle_desk_get_curr() - Resolve args->desk from the current desk toggle
 * @base: task base directory
 * @args: env/desk/task selection; args->desk is left untouched if
 *        already set, otherwise filled in from the on-disk toggle
 *
 * Return: ETEC_OK if args->desk was already set or was resolved from
 * the toggle, ETEC_TOGG_DESK_GET_CURR if it was unset and no current
 * desk toggle exists
 */
int toggle_desk_get_curr(char *base, tec_arg_t *args)
{
    if (!args->desk && !(args->desk = desk_get_curr(base, args)))
        return ETEC_TOGG_DESK_GET_CURR;
    return ETEC_OK;
}

/**
 * toggle_desk_get_prev() - Resolve args->desk from the previous desk toggle
 * @base: task base directory
 * @args: env/desk/task selection; args->desk is left untouched if
 *        already set, otherwise filled in from the on-disk toggle
 *
 * Return: ETEC_OK if args->desk was already set or was resolved from
 * the toggle, ETEC_TOGG_DESK_GET_PREV if it was unset and no previous
 * desk toggle exists
 */
int toggle_desk_get_prev(char *base, tec_arg_t *args)
{
    if (!args->desk && !(args->desk = desk_get_prev(base, args)))
        return ETEC_TOGG_DESK_GET_PREV;
    return ETEC_OK;
}

/**
 * toggle_task_get_curr() - Resolve args->task from the current task toggle
 * @base: task base directory
 * @args: env/desk/task selection; args->task is left untouched if
 *        already set, otherwise filled in from the on-disk toggle
 *
 * Return: ETEC_OK if args->task was already set or was resolved from
 * the toggle, ETEC_TOGG_TASK_GET_CURR if it was unset and no current
 * task toggle exists
 */
int toggle_task_get_curr(char *base, tec_arg_t *args)
{
    if (!args->task && !(args->task = task_get_curr(base, args)))
        return ETEC_TOGG_TASK_GET_CURR;
    return ETEC_OK;
}

/**
 * toggle_task_get_prev() - Resolve args->task from the previous task toggle
 * @base: task base directory
 * @args: env/desk/task selection; args->task is left untouched if
 *        already set, otherwise filled in from the on-disk toggle
 *
 * Return: ETEC_OK if args->task was already set or was resolved from
 * the toggle, ETEC_TOGG_TASK_GET_PREV if it was unset and no previous
 * task toggle exists
 */
int toggle_task_get_prev(char *base, tec_arg_t *args)
{
    if (!args->task && !(args->task = task_get_prev(base, args)))
        return ETEC_TOGG_TASK_GET_PREV;
    return ETEC_OK;
}

/**
 * toggle_task_is_curr() - Check whether args->task is the current task
 * @base: task base directory
 * @args: env/desk/task selection; args->task is compared against the
 *        on-disk current toggle
 *
 * Return: true if the current task toggle equals args->task, false if
 * there is no current toggle or it differs
 */
bool toggle_task_is_curr(char *base, tec_arg_t *args)
{
    char *task;

    /* There is no current task ID - immediately return false.  */
    if ((task = task_get_curr(base, args)) == NULL)
        return false;
    return !strcmp(task, args->task);
}

/**
 * toggle_task_is_prev() - Check whether args->task is the previous task
 * @base: task base directory
 * @args: env/desk/task selection; args->task is compared against the
 *        on-disk previous toggle
 *
 * Return: true if the previous task toggle equals args->task, false
 * if there is no previous toggle or it differs
 */
bool toggle_task_is_prev(char *base, tec_arg_t *args)
{
    char *task;

    /* There is no previous task ID - immediately return false.  */
    if ((task = task_get_prev(base, args)) == NULL)
        return false;
    return !strcmp(task, args->task);
}

/**
 * toggle_env_set_curr() - Make args->env the current environment toggle
 * @base: task base directory
 * @args: env/desk/task selection; args->env becomes the new "curr"
 *
 * The previous "curr" value (if any and if different from args->env)
 * is shifted into "prev" before the toggle file is rewritten, so
 * repeated sets of the same value do not create duplicate/no-op
 * writes.
 *
 * Return: always 0
 */
int toggle_env_set_curr(char *base, tec_arg_t *args)
{
    char *curr, *prev;
    tec_unit_t *toggles;

    toggles = NULL;
    curr = args->env;
    prev = env_get_curr(base, args);

    toggles = tec_unit_add(toggles, "curr", curr);
    if (prev)
        toggles = tec_unit_add(toggles, "prev", prev);

    /* Prevent duplicates in toggles.  */
    if (!(prev && strcmp(curr, prev) == 0))
        tec_unit_save(path_env_toggle(base, args), toggles);
    tec_unit_free(toggles);
    return 0;
}

/**
 * toggle_env_unset_prev() - Clear the previous environment toggle
 * @base: task base directory
 * @args: env/desk/task selection identifying the toggle file's location
 *
 * Rewrites the toggle file keeping only the current value (if any)
 * and dropping "prev".
 *
 * Return: 1 if there was no previous toggle to clear (no-op), or the
 * status of tec_unit_save() (0 on success) otherwise
 */
int toggle_env_unset_prev(char *base, tec_arg_t *args)
{
    int status;
    char *curr, *prev;
    tec_unit_t *toggles;

    toggles = NULL;
    curr = env_get_curr(base, args);
    prev = env_get_prev(base, args);

    if (prev == NULL)
        return 1;               // do nothing
    if (curr != NULL) {
        // rewrite curr with the same value
        toggles = tec_unit_add(toggles, "curr", curr);
    }
    status = tec_unit_save(path_env_toggle(base, args), toggles);
    tec_unit_free(toggles);
    return status;
}

/**
 * toggle_desk_set_curr() - Make args->desk the current desk toggle
 * @base: task base directory
 * @args: env/desk/task selection; args->desk becomes the new "curr"
 *
 * The previous "curr" value (if any and if different from args->desk)
 * is shifted into "prev" before the toggle file is rewritten, so
 * repeated sets of the same value do not create duplicate/no-op
 * writes.
 *
 * Return: always 0
 */
int toggle_desk_set_curr(char *base, tec_arg_t *args)
{
    char *curr, *prev;
    tec_unit_t *toggles;

    toggles = NULL;
    curr = args->desk;
    prev = desk_get_curr(base, args);

    toggles = tec_unit_add(toggles, "curr", curr);
    if (prev)
        toggles = tec_unit_add(toggles, "prev", prev);

    /* Prevent duplicates in toggles.  */
    if (!(prev && strcmp(curr, prev) == 0))
        tec_unit_save(path_desk_toggle(base, args), toggles);
    tec_unit_free(toggles);
    return 0;
}

/**
 * toggle_task_set_curr() - Make args->task the current task toggle
 * @base: task base directory
 * @args: env/desk/task selection; args->task becomes the new "curr"
 *
 * The previous "curr" value (if any and if different from args->task)
 * is shifted into "prev" before the toggle file is rewritten, so
 * repeated sets of the same value do not create duplicate/no-op
 * writes.
 *
 * Return: always ETEC_OK
 */
int toggle_task_set_curr(char *base, tec_arg_t *args)
{
    char *curr, *prev;
    tec_unit_t *toggles;

    toggles = NULL;
    curr = args->task;
    prev = task_get_curr(base, args);

    toggles = tec_unit_add(toggles, "curr", curr);
    if (prev)
        toggles = tec_unit_add(toggles, "prev", prev);

    /* Prevent duplicates in toggles.  */
    if (!(prev && strcmp(curr, prev) == 0))
        tec_unit_save(path_task_toggle(base, args), toggles);
    tec_unit_free(toggles);
    return ETEC_OK;
}

/**
 * toggle_task_unset_curr() - Clear the current task toggle, promoting prev if present
 * @base: task base directory
 * @args: env/desk/task selection identifying the toggle file's location
 *
 * If there is no current task toggle, this is a no-op. Otherwise the
 * previous task value (if any) is promoted to become the new
 * "curr", and the toggle file is rewritten (with no "curr" entry at
 * all if there was no previous value either).
 *
 * Return: 1 if there was no current toggle to clear (no-op), or the
 * status of tec_unit_save() (0 on success) otherwise
 */
int toggle_task_unset_curr(char *base, tec_arg_t *args)
{
    int status;
    char *curr, *prev;
    tec_unit_t *toggles;

    toggles = NULL;
    curr = task_get_curr(base, args);
    prev = task_get_prev(base, args);

    if (curr == NULL)
        return 1;
    else if (prev != NULL)
        toggles = tec_unit_add(toggles, "curr", prev);

    status = tec_unit_save(path_task_toggle(base, args), toggles);
    tec_unit_free(toggles);
    return status;
}

/**
 * toggle_task_unset_prev() - Clear the previous task toggle
 * @base: task base directory
 * @args: env/desk/task selection identifying the toggle file's location
 *
 * Rewrites the toggle file keeping only the current value (if any)
 * and dropping "prev".
 *
 * Return: ETEC_TOGG_TASK_UNSET_PREV if there was no previous toggle
 * to clear (no-op), or the status of tec_unit_save() (0 on success)
 * otherwise
 */
int toggle_task_unset_prev(char *base, tec_arg_t *args)
{
    int status;
    char *curr, *prev;
    tec_unit_t *toggles;

    toggles = NULL;
    curr = task_get_curr(base, args);
    prev = task_get_prev(base, args);

    if (prev == NULL)
        return ETEC_TOGG_TASK_UNSET_PREV;       // do nothing
    if (curr != NULL) {
        // rewrite curr with the same value
        toggles = tec_unit_add(toggles, "curr", curr);
    }
    status = tec_unit_save(path_task_toggle(base, args), toggles);
    tec_unit_free(toggles);
    return status;
}

/*
 * Update task toggles after a task is renamed.
 * If old_id matches curr or prev, update it to new_id.
 * args must have env and desk set for the location.
 */
/**
 * toggle_task_update() - Rewrite a task's curr/prev toggle after a rename
 * @base: task base directory
 * @args: env/desk/task selection identifying the toggle file's
 *        location; args->env and args->desk must be set
 * @old_id: old task ID to match against curr/prev
 * @new_id: new task ID to replace @old_id with
 *
 * If the current or previous toggle value equals @old_id, replaces it
 * with @new_id and rewrites the toggle file with both (updated)
 * values. Does nothing if neither curr nor prev matched @old_id.
 *
 * Return: always 0
 */
int toggle_task_update(char *base, tec_arg_t *args,
                       const char *old_id, const char *new_id)
{
    char *curr, *prev;
    tec_unit_t *toggles;
    int changed = 0;

    toggles = NULL;
    curr = task_get_curr(base, args);
    prev = task_get_prev(base, args);

    /* Check if old_id matches curr or prev and update */
    if (curr && strcmp(curr, old_id) == 0) {
        curr = (char *)new_id;
        changed = 1;
    }
    if (prev && strcmp(prev, old_id) == 0) {
        prev = (char *)new_id;
        changed = 1;
    }

    if (!changed)
        return 0;               /* Nothing to update */

    if (curr)
        toggles = tec_unit_add(toggles, "curr", curr);
    if (prev)
        toggles = tec_unit_add(toggles, "prev", prev);

    if (toggles) {
        tec_unit_save(path_task_toggle(base, args), toggles);
        tec_unit_free(toggles);
    }
    return 0;
}

/*
 * Clear a task from toggles when it's moved to a different desk/env.
 * If task matches curr, promote prev to curr and clear prev.
 * If task matches prev, just clear prev.
 * args must have env and desk set for the source location.
 */
/**
 * toggle_task_clear() - Remove a specific task from a location's toggles
 * @base: task base directory
 * @args: env/desk/task selection identifying the source toggle
 *        file's location; args->env and args->desk must be set
 * @task: task ID being moved/removed, matched against curr and prev
 *
 * If @task matches the current toggle, prev is promoted to curr and
 * prev is cleared. If @task matches only the previous toggle, prev is
 * simply cleared. If @task matches neither, the toggle file is left
 * untouched.
 *
 * Return: always 0
 */
int toggle_task_clear(char *base, tec_arg_t *args, const char *task)
{
    char *curr, *prev;
    tec_unit_t *toggles;
    int changed = 0;

    toggles = NULL;
    curr = task_get_curr(base, args);
    prev = task_get_prev(base, args);

    if (curr && strcmp(curr, task) == 0) {
        /* Current task is being moved, promote prev to curr */
        curr = prev;
        prev = NULL;
        changed = 1;
    } else if (prev && strcmp(prev, task) == 0) {
        /* Previous task is being moved, just clear it */
        prev = NULL;
        changed = 1;
    }

    if (!changed) {
        TEC_LOG_D("%s: NOT HAPPENED\n", __func__);
        return 0;
    }

    if (curr)
        toggles = tec_unit_add(toggles, "curr", curr);
    if (prev)
        toggles = tec_unit_add(toggles, "prev", prev);

    tec_unit_save(path_task_toggle(base, args), toggles);
    tec_unit_free(toggles);
    return 0;
}
