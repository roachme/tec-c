#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <sys/wait.h>

#include "tec.h"
#include "aux/config.h"

/**
 * pgn_path() - Build the path to a plugin's binary
 * @path: destination buffer, must hold at least PATH_MAX + 1 bytes
 * @cfg: active configuration, providing the plugin directory
 * @name: plugin/command name
 *
 * Writes "<pgn dir>/<name>/<name>" into @path.
 *
 * Return: EXIT_SUCCESS on success, EXIT_FAILURE if the path would overflow
 */
static int pgn_path(char *path, tec_cfg_t *cfg, const char *name)
{
    int len = snprintf(path, PATH_MAX + 1, "%s/%s/%s", cfg->base.pgn, name,
                       name);

    return (len < 0 || len >= PATH_MAX + 1) ? EXIT_FAILURE : EXIT_SUCCESS;
}

/**
 * pgn_argv_build() - Build the argv array to exec a plugin binary
 * @path: plugin binary path, used as argv[0]
 * @argvec: remaining user arguments to forward to the plugin, already
 *          offset past the plugin name
 * @cfg: active configuration, providing "-T task -P pgn" to forward
 *
 * Allocates a NULL-terminated argv array: "<path> -T <task> -P <pgn>
 * <argvec...>". No shell is involved, so every element is passed to
 * the plugin verbatim as a single argument, regardless of spaces or
 * shell metacharacters it may contain. The array itself is
 * heap-allocated and owned by the caller; its string elements are
 * borrowed from @path, @cfg and @argvec and must outlive it.
 *
 * Return: heap-allocated argv array, or NULL on allocation failure
 */
static char **pgn_argv_build(char *path, tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    int n = 0;
    char **argv = malloc(sizeof(char *) * (5 + argvec->used + 1));

    if (!argv)
        return NULL;

    argv[n++] = path;
    argv[n++] = "-T";
    argv[n++] = cfg->base.task;
    argv[n++] = "-P";
    argv[n++] = cfg->base.pgn;
    for (int i = 0; i < argvec->used; ++i)
        argv[n++] = argvec->argv[i];
    argv[n] = NULL;
    return argv;
}

/**
 * pgn_exec() - Fork and exec a plugin binary, waiting for it to finish
 * @argv: NULL-terminated argv array; argv[0] is the executable path,
 *        used both to exec and to name the plugin in error messages
 *
 * Return: EXIT_SUCCESS if the plugin exited with status 0, EXIT_FAILURE
 * otherwise (including fork/exec/wait failures)
 */
static int pgn_exec(char *const argv[])
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0)
        return TEC_LOG_E("pgn: fork failed: %s", strerror(errno));

    if (pid == 0) {
        execv(argv[0], argv);
        TEC_LOG_E("pgn: exec '%s' failed: %s", argv[0], strerror(errno));
        _exit(127);
    }

    if (waitpid(pid, &status, 0) < 0)
        return TEC_LOG_E("pgn: waitpid failed: %s", strerror(errno));

    return (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS)
        ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**
 * tec_cli_pgn() - Run an external plugin binary as if it were a builtin command
 * @argvec: parsed argv; argv[0] is the plugin name, the rest are forwarded
 *          to the plugin as-is
 * @cfg: active configuration, providing the plugin and task directories
 *
 * Resolves the plugin's binary path with pgn_path(), builds its argv
 * with pgn_argv_build(), and runs it directly via fork()+execv() (no
 * shell involved), so plugin arguments cannot be reinterpreted as
 * shell syntax.
 *
 * Return: the value of TEC_LOG_E() if the path or argv couldn't be built,
 * otherwise EXIT_SUCCESS if the plugin process exited with EXIT_SUCCESS,
 * else EXIT_FAILURE
 */
int tec_cli_pgn(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    char path[PATH_MAX + 1];
    char **argv;
    int status;
    char *name = argvec->argv[0];

    argvec_offset(argvec, 1);

    if (pgn_path(path, cfg, name))
        return TEC_LOG_E("pgn: base generation buffer overflow");

    if (!(argv = pgn_argv_build(path, argvec, cfg)))
        return TEC_LOG_E("pgn: memory allocation failed");

    TEC_LOG_D("pgn: exec %s", path);
    status = pgn_exec(argv);
    free(argv);
    return status;
}
