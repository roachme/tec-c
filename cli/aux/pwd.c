#include <stdio.h>
#include "pwd.h"
#include "log.h"
#include "config.h"

/**
 * pwd_write() - Write the target directory line to the $PWD file
 * @args: env/desk/task selection identifying the target directory;
 *        any NULL field is treated as an empty path component
 * @path: optional subdirectory appended after the task ID, or NULL
 *        for none
 *
 * Overwrites PWDFILE with a single line of the form
 * "<taskbase>/<env>/<desk>/<task>[/<path>]\n" so the shell wrapper can
 * `cd` there after tec exits.
 *
 * Return: 0 on success, 1 if PWDFILE could not be opened for writing,
 * or whatever fclose() returns (non-zero) if closing it failed
 */
static int pwd_write(tec_arg_t *args, const char *path)
{
    FILE *fp;
    char *taskdir = teccfg.base.task;
    const char *fmt = "%s/%s/%s/%s%s%s\n";
    const char *fmtdebug = "pwd: env='%s', desk='%s', task='%s', path='%s'";

    args->env = args->env == NULL ? "" : args->env;
    args->desk = args->desk == NULL ? "" : args->desk;
    args->task = args->task == NULL ? "" : args->task;
    path = path == NULL ? "" : path;

    if ((fp = fopen(PWDFILE, "w"))) {
        TEC_LOG_D(fmtdebug, args->env, args->desk, args->task, path);
        fprintf(fp, fmt, taskdir, args->env, args->desk, args->task,
                path[0] ? "/" : "", path);
        return fclose(fp);
    }
    return 1;
}

/**
 * tec_cli_pwd_set() - Record the current env/desk/task as the shell's next directory
 * @args: env/desk/task selection to write out
 *
 * Return: 0 on success, non-zero on failure (see pwd_write())
 */
int tec_cli_pwd_set(tec_arg_t *args)
{
    return pwd_write(args, NULL);
}

/**
 * tec_cli_pwd_set_path() - Record env/desk/task plus a subdirectory as the shell's next directory
 * @args: env/desk/task selection to write out
 * @path: subdirectory inside the task directory to append
 *
 * Return: 0 on success, non-zero on failure (see pwd_write())
 */
int tec_cli_pwd_set_path(tec_arg_t *args, const char *path)
{
    return pwd_write(args, path);
}

/**
 * tec_cli_pwd_unset() - Clear the $PWD file so the shell wrapper does not `cd`
 *
 * Truncates PWDFILE to empty by reopening it in write mode.
 *
 * Return: 0 on success, 1 if PWDFILE could not be opened, or whatever
 * fclose() returns (non-zero) if closing it failed
 */
int tec_cli_pwd_unset()
{
    FILE *fp;

    if ((fp = fopen(PWDFILE, "w")) == NULL)
        return 1;
    return fclose(fp);
}
