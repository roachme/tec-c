#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

#include "log.h"
#include "hook.h"
#include "config.h"
#include "errno.h"
#include "../../lib/libtec.h"

static char pathname[PATH_MAX + 1];

/**
 * _hook_cmd() - Build the shell command line to invoke a plugin hook
 * @args: env/desk/task selection to pass to the plugin
 * @name: plugin directory/binary name
 * @cmd: plugin subcommand to invoke
 *
 * Formats "<pgnbase>/<name>/<name> -T <taskbase> <cmd> -e <env> -d
 * <desk> <task>" into a static buffer.
 *
 * Return: pointer to a static buffer holding the built command line;
 * overwritten on the next call to this function
 */
static char *_hook_cmd(tec_arg_t *args, char *name, char *cmd)
{
    const char *fmt = "%s/%s/%s -T %s %s -e %s -d %s %s";
    sprintf(pathname, fmt, teccfg.base.pgn, name, name, teccfg.base.task,
            cmd, args->env, args->desk, args->task);
    return pathname;
}

/**
 * hook_action() - Run every configured hook matching @cmd
 * @args: env/desk/task selection passed through to each hook
 * @cmd: hook command name to match against configured hooks (e.g. "add")
 *
 * No-op if hooks are globally disabled (teccfg.opts.hook == false).
 * Otherwise walks teccfg.hooks and, for each entry whose cmd matches
 * @cmd, shells out via system() to run it, continuing even if one
 * invocation fails so all matching hooks get a chance to run.
 *
 * Return: ETEC_OK if every matching hook exited successfully (or
 * hooks are disabled/none matched), ETEC_HOOK_EXEC if at least one
 * hook's system() call failed
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
            char *_cmd = _hook_cmd(args, hooks->pgname, hooks->pgncmd);
            TEC_LOG_D(_cmd);
            status = system(_cmd) == EXIT_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
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
 * whose cmd matches @cmd, shells out via popen() and parses every
 * line of its stdout into *@units via tec_unit_parse(). Continues to
 * the next hook even if one invocation fails to open or exits
 * non-zero.
 *
 * Return: EXIT_SUCCESS if hooks are disabled or every matching hook's
 * pipe opened and closed successfully, ETEC_HOOK_EXEC if at least one
 * matching hook failed to open or exited non-zero
 */
int hook_cat(tec_unit_t **units, tec_arg_t *args, char *cmd)
{
    FILE *pipe;
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

        char *hook_cmd = _hook_cmd(args, hooks->pgname, hooks->pgncmd);
        TEC_LOG_D(hook_cmd);
        if (!(pipe = popen(hook_cmd, "r"))) {
            // TODO: add quiet option and show error message of plugin
            retcode = EXIT_FAILURE;
            continue;
        }
        while (fgets(line, BUFSIZ, pipe))
            *units = tec_unit_parse(*units, line);
        status = pclose(pipe) == EXIT_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
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
