#include <stdio.h>
#include <limits.h>
#include <stdarg.h>

#include "path.h"

#define ENVFMT_DIR          "%s/%s"
#define DESKFMT_DIR         ENVFMT_DIR "/" "%s"
#define TASKFMT_DIR         DESKFMT_DIR "/" "%s"

#define ENVFMT_DB           ENVFMT_DIR "/" ".tec"
#define DESKFMT_DB          DESKFMT_DIR "/" ".tec"
#define TASKFMT_DB          TASKFMT_DIR "/" ".tec"

#define ENVFMT_UNIT         ENVFMT_DB "/" "units"
#define DESKFMT_UNIT        DESKFMT_DB "/" "units"
#define TASKFMT_UNIT        TASKFMT_DB "/" "units"

/**
 * path_generic() - format a path into @buf using a printf-style format
 * @buf: destination buffer to write the formatted path into
 * @bufsiz: usable size of @buf, not counting the trailing NUL
 * @fmt: printf-style format string describing the path layout
 * @...: arguments consumed by @fmt
 *
 * Return: @buf on success, or NULL if the formatted string would not
 * fit within @bufsiz.
 */
static char *path_generic(char *buf, int bufsiz, const char *fmt, ...)
{
    int len;
    va_list arg;

    va_start(arg, fmt);
    len = vsnprintf(buf, bufsiz, fmt, arg);
    va_end(arg);
    return len >= bufsiz ? NULL : buf;
}

/**
 * path_env_dir() - build the directory path of an environment
 * @taskdir: root directory of the task database
 * @args: identifiers of the environment (uses @args->env)
 *
 * Return: a pointer to a static buffer holding the path, valid until
 * the next call to any path_* function; NULL if the path would exceed
 * PATH_MAX.
 */
char *path_env_dir(const char *taskdir, const tec_arg_t *args)
{
    const char *fmt = ENVFMT_DIR;
    static char pathname[PATH_MAX + 1];
    return path_generic(pathname, PATH_MAX, fmt, taskdir, args->env);
}

/**
 * path_env_db() - build the path of an environment's .tec metadata directory
 * @taskdir: root directory of the task database
 * @args: identifiers of the environment (uses @args->env)
 *
 * Return: a pointer to a static buffer holding the path, valid until
 * the next call to any path_* function; NULL if the path would exceed
 * PATH_MAX.
 */
char *path_env_db(const char *taskdir, const tec_arg_t *args)
{
    const char *fmt = ENVFMT_DB;
    static char pathname[PATH_MAX + 1];
    return path_generic(pathname, PATH_MAX, fmt, taskdir, args->env);
}

/**
 * path_env_unit() - build the path of an environment's units file
 * @taskdir: root directory of the task database
 * @args: identifiers of the environment (uses @args->env)
 *
 * Return: a pointer to a static buffer holding the path, valid until
 * the next call to any path_* function; NULL if the path would exceed
 * PATH_MAX.
 */
char *path_env_unit(const char *taskdir, const tec_arg_t *args)
{
    const char *fmt = ENVFMT_UNIT;
    static char pathname[PATH_MAX + 1];
    return path_generic(pathname, PATH_MAX, fmt, taskdir, args->env);
}

/**
 * path_desk_dir() - build the directory path of a desk
 * @taskdir: root directory of the task database
 * @args: identifiers of the desk (uses @args->env and @args->desk)
 *
 * Return: a pointer to a static buffer holding the path, valid until
 * the next call to any path_* function; NULL if the path would exceed
 * PATH_MAX.
 */
char *path_desk_dir(const char *taskdir, const tec_arg_t *args)
{
    const char *fmt = DESKFMT_DIR;
    static char pathname[PATH_MAX + 1];
    return path_generic(pathname, PATH_MAX, fmt, taskdir, args->env,
                        args->desk);
}

/**
 * path_desk_db() - build the path of a desk's .tec metadata directory
 * @taskdir: root directory of the task database
 * @args: identifiers of the desk (uses @args->env and @args->desk)
 *
 * Return: a pointer to a static buffer holding the path, valid until
 * the next call to any path_* function; NULL if the path would exceed
 * PATH_MAX.
 */
char *path_desk_db(const char *taskdir, const tec_arg_t *args)
{
    const char *fmt = DESKFMT_DB;
    static char pathname[PATH_MAX + 1];
    return path_generic(pathname, PATH_MAX, fmt, taskdir, args->env,
                        args->desk);
}

/**
 * path_desk_unit() - build the path of a desk's units file
 * @taskdir: root directory of the task database
 * @args: identifiers of the desk (uses @args->env and @args->desk)
 *
 * Return: a pointer to a static buffer holding the path, valid until
 * the next call to any path_* function; NULL if the path would exceed
 * PATH_MAX.
 */
char *path_desk_unit(const char *taskdir, const tec_arg_t *args)
{
    const char *fmt = DESKFMT_UNIT;
    static char pathname[PATH_MAX + 1];
    return path_generic(pathname, PATH_MAX, fmt, taskdir, args->env,
                        args->desk);
}

/**
 * path_task_dir() - build the directory path of a task
 * @taskdir: root directory of the task database
 * @args: identifiers of the task (uses @args->env, @args->desk, @args->task)
 *
 * Return: a pointer to a static buffer holding the path, valid until
 * the next call to any path_* function; NULL if the path would exceed
 * PATH_MAX.
 */
char *path_task_dir(const char *taskdir, const tec_arg_t *args)
{
    const char *fmt = TASKFMT_DIR;
    static char pathname[PATH_MAX + 1];
    return path_generic(pathname, PATH_MAX, fmt, taskdir, args->env, args->desk,
                        args->task);
}

/**
 * path_task_db() - build the path of a task's .tec metadata directory
 * @taskdir: root directory of the task database
 * @args: identifiers of the task (uses @args->env, @args->desk, @args->task)
 *
 * Return: a pointer to a static buffer holding the path, valid until
 * the next call to any path_* function; NULL if the path would exceed
 * PATH_MAX.
 */
char *path_task_db(const char *taskdir, const tec_arg_t *args)
{
    const char *fmt = TASKFMT_DB;
    static char pathname[PATH_MAX + 1];
    return path_generic(pathname, PATH_MAX, fmt, taskdir, args->env, args->desk,
                        args->task);
}

/**
 * path_task_unit() - build the path of a task's units file
 * @taskdir: root directory of the task database
 * @args: identifiers of the task (uses @args->env, @args->desk, @args->task)
 *
 * Return: a pointer to a static buffer holding the path, valid until
 * the next call to any path_* function; NULL if the path would exceed
 * PATH_MAX.
 */
char *path_task_unit(const char *taskdir, const tec_arg_t *args)
{
    const char *fmt = TASKFMT_UNIT;
    static char pathname[PATH_MAX + 1];
    return path_generic(pathname, PATH_MAX, fmt, taskdir, args->env, args->desk,
                        args->task);
}
