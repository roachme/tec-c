#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

#include "log.h"
#include "hook.h"
#include "config.h"
#include "errno.h"
#include "../../lib/libtec.h"

static char pathname[PATH_MAX + 1];
static char *hook_argv[10];

/**
 * hook_path() - Resolve a hook's plugin binary path
 * @name: plugin directory/binary name
 *
 * Formats "<pgnbase>/<name>/<name>" into a static buffer.
 *
 * Return: pointer to a static buffer holding the path (overwritten on
 * the next call), or NULL if the path would overflow the buffer
 */
static char *hook_path(char *name)
{
    int len = snprintf(pathname, sizeof(pathname), "%s/%s/%s",
                       teccfg.base.pgn, name, name);

    return (len < 0 || (size_t)len >= sizeof(pathname)) ? NULL : pathname;
}

/**
 * hook_argv_build() - Build the argv array to exec a plugin hook
 * @path: plugin binary path, used as argv[0]
 * @args: env/desk/task selection to pass to the plugin
 * @cmd: plugin subcommand to invoke
 *
 * Fills a static NULL-terminated argv array: "<path> -T <taskbase>
 * <cmd> -e <env> -d <desk> <task>". No shell is involved, so each of
 * @args' fields reaches the plugin as a single argument verbatim,
 * regardless of spaces or shell metacharacters it may contain.
 *
 * Return: pointer to a static argv array, overwritten on the next call
 */
static char **hook_argv_build(char *path, tec_arg_t *args, char *cmd)
{
    hook_argv[0] = path;
    hook_argv[1] = "-T";
    hook_argv[2] = teccfg.base.task;
    hook_argv[3] = cmd;
    hook_argv[4] = "-e";
    hook_argv[5] = args->env;
    hook_argv[6] = "-d";
    hook_argv[7] = args->desk;
    hook_argv[8] = args->task;
    hook_argv[9] = NULL;
    return hook_argv;
}

/**
 * hook_exec() - Fork and exec a plugin hook, waiting for it to finish
 * @argv: NULL-terminated argv array; argv[0] is the executable path
 *
 * Return: EXIT_SUCCESS if the hook exited with status 0, EXIT_FAILURE
 * otherwise (including fork/exec/wait failures)
 */
static int hook_exec(char *const argv[])
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0)
        return TEC_LOG_E("hook: fork failed: %s", strerror(errno));

    if (pid == 0) {
        execv(argv[0], argv);
        TEC_LOG_E("hook: exec '%s' failed: %s", argv[0], strerror(errno));
        _exit(127);
    }

    if (waitpid(pid, &status, 0) < 0)
        return TEC_LOG_E("hook: waitpid failed: %s", strerror(errno));

    return (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS)
        ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**
 * hook_popen() - Fork and exec a plugin hook, capturing its stdout
 * @argv: NULL-terminated argv array; argv[0] is the executable path
 * @pid_out: filled with the child's pid on success, for hook_pclose()
 *
 * Return: a FILE open for reading the hook's stdout (as popen(..., "r")
 * would return), or NULL on failure
 */
static FILE *hook_popen(char *const argv[], pid_t *pid_out)
{
    int fd[2];
    pid_t pid;

    if (pipe(fd) < 0)
        return NULL;

    pid = fork();
    if (pid < 0) {
        close(fd[0]);
        close(fd[1]);
        return NULL;
    }

    if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        execv(argv[0], argv);
        TEC_LOG_E("hook: exec '%s' failed: %s", argv[0], strerror(errno));
        _exit(127);
    }

    close(fd[1]);
    *pid_out = pid;
    return fdopen(fd[0], "r");
}

/**
 * hook_pclose() - Close a stream from hook_popen() and reap its child
 * @pipe: stream returned by hook_popen()
 * @pid: child pid returned alongside it
 *
 * Return: EXIT_SUCCESS if the child exited with status 0, EXIT_FAILURE
 * otherwise
 */
static int hook_pclose(FILE *pipe, pid_t pid)
{
    int status;

    fclose(pipe);
    if (waitpid(pid, &status, 0) < 0)
        return EXIT_FAILURE;

    return (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS)
        ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**
 * hook_action() - Run every configured hook matching @cmd
 * @args: env/desk/task selection passed through to each hook
 * @cmd: hook command name to match against configured hooks (e.g. "add")
 *
 * No-op if hooks are globally disabled (teccfg.opts.hook == false).
 * Otherwise walks teccfg.hooks and, for each entry whose cmd matches
 * @cmd, execs it directly (no shell involved) via hook_exec(),
 * continuing even if one invocation fails so all matching hooks get a
 * chance to run.
 *
 * Return: ETEC_OK if every matching hook exited successfully (or
 * hooks are disabled/none matched), ETEC_HOOK_EXEC if at least one
 * hook failed to build or run
 */
int hook_action(tec_arg_t *args, char *cmd)
{
    int retcode, status;
    struct tec_hook *hooks = teccfg.hooks;

    retcode = status = ETEC_OK;

    /* Execute hooks only if they are enabled.  */
    if (teccfg.opts.hook == false)
        return 0;

    for (; hooks; hooks = hooks->next) {
        if (strcmp(cmd, hooks->cmd) == 0) {
            char *path = hook_path(hooks->pgname);

            if (!path) {
                retcode = ETEC_HOOK_EXEC;
                continue;
            }

            TEC_LOG_D("hook: exec %s", path);
            status = hook_exec(hook_argv_build(path, args, hooks->pgncmd));
            retcode = status == ETEC_OK ? retcode : status;
        }
    }

    return retcode == ETEC_OK ? ETEC_OK : ETEC_HOOK_EXEC;
}

/**
 * hook_cat() - Run every configured hook matching @cmd and collect its output
 * @units: linked list to append parsed key/value output lines to
 * @args: env/desk/task selection passed through to each hook
 * @cmd: hook command name to match against configured hooks (e.g. "cat")
 *
 * No-op (success) if hooks are globally disabled
 * (teccfg.opts.hook == false). Otherwise, for each configured hook
 * whose cmd matches @cmd, execs it directly (no shell involved) via
 * hook_popen() and parses every line of its stdout into *@units via
 * tec_unit_parse(). Continues to the next hook even if one invocation
 * fails to start or exits non-zero.
 *
 * Return: EXIT_SUCCESS if hooks are disabled or every matching hook's
 * pipe opened and closed successfully, ETEC_HOOK_EXEC if at least one
 * matching hook failed to open or exited non-zero
 */
int hook_cat(tec_unit_t **units, tec_arg_t *args, char *cmd)
{
    FILE *pipe;
    pid_t pid;
    int retcode, status;
    char line[BUFSIZ + 1] = { 0 };
    struct tec_hook *hooks = teccfg.hooks;

    retcode = status = ETEC_OK;

    /* Execute hooks only if they are enabled.  */
    if (teccfg.opts.hook == false)
        return EXIT_SUCCESS;

    for (; hooks; hooks = hooks->next) {
        if (strcmp(hooks->cmd, cmd) != 0)
            continue;

        char *path = hook_path(hooks->pgname);

        if (!path) {
            retcode = EXIT_FAILURE;
            continue;
        }

        TEC_LOG_D("hook: exec %s", path);
        if (!
            (pipe =
             hook_popen(hook_argv_build(path, args, hooks->pgncmd), &pid))) {
            // TODO: add quiet option and show error message of plugin
            retcode = EXIT_FAILURE;
            continue;
        }
        while (fgets(line, BUFSIZ, pipe))
            *units = tec_unit_parse(*units, line);
        status = hook_pclose(pipe, pid);
        retcode = status == EXIT_SUCCESS ? retcode : status;
    }
    return retcode == ETEC_OK ? ETEC_OK : ETEC_HOOK_EXEC;
}

/*
char *hook_list(struct tec_hook *hooks, char *pgnout, char *env, char *task)
{
    FILE *pipe;
    char *prefix = "  ";
    char line[BUFSIZ + 1] = { 0 };

    // Execute hooks only if they are enabled.
    if (teccfg.opts.hook == true)
        return 0;

    for (; hooks; hooks = hooks->next) {
        if (strcmp(hooks->cmd, "list") != 0)
            continue;

        if ((pipe =
             popen(genpath_pgn(env, task, hooks->pgname, hooks->pgncmd),
                   "r")) == NULL) {
            return NULL;
        }
        // NOTE: gotta get a single word
        if (fgets(line, BUFSIZ, pipe)) {
            line[strcspn(line, "\n")] = 0;
            strcat(pgnout, prefix);
            strcat(pgnout, line);
        }

        pclose(pipe);
    }

    if (pgnout[1] == ' ') {
        pgnout[1] = '[';
        strcat(pgnout, "]");
    }
    //return ETEC_OK;
    return pgnout;
}
*/
