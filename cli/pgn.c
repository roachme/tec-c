#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tec.h"
#include "aux/config.h"

typedef struct tec_pgn_cmd {
    size_t size;
    size_t offset;
    char name[CMDSIZ + 1];
    char cmd[BUFSIZ + 1];
} tec_pgn_cmd_t;

/**
 * pgn_cmd_init() - Initialize a plugin shell-command builder
 * @cmd: the command buffer to initialize
 * @name: plugin/command name, copied into @cmd->name
 */
static void pgn_cmd_init(tec_pgn_cmd_t *cmd, char *name)
{
    strcpy(cmd->name, name);
    cmd->cmd[0] = '\0';
    cmd->offset = 0;
    cmd->size = sizeof(cmd->cmd);
}

/**
 * pgn_cmd_add_path() - Append the plugin binary's path to the command buffer
 * @cmd: command buffer being built; ->offset is advanced past what was
 *       written so later appends continue from there
 * @cfg: active configuration, providing the plugin directory
 *
 * Writes "<pgn dir>/<name>/<name>" using @cmd->name as the plugin name.
 *
 * Return: EXIT_SUCCESS on success, EXIT_FAILURE if the buffer is too small
 */
static int pgn_cmd_add_path(tec_pgn_cmd_t *cmd, tec_cfg_t *cfg)
{
    size_t len;
    const char *name = cmd->name;
    const char *fmt = "%s/%s/%s";
    char *cmdptr = cmd->cmd + cmd->offset;

    len = snprintf(cmdptr, cmd->size, fmt, cfg->base.pgn, name, name);
    if (len >= cmd->size)
        return EXIT_FAILURE;

    cmd->offset = strlen(cmd->cmd);
    return EXIT_SUCCESS;
}

/**
 * pgn_cmd_add_opts() - Append the "-T task -P pgn" options to the command buffer
 * @cmd: command buffer being built; ->offset is advanced past what was
 *       written so later appends continue from there
 * @cfg: active configuration, providing the task and plugin directories
 *       to forward to the plugin process
 *
 * Return: EXIT_SUCCESS on success, EXIT_FAILURE if the buffer is too small
 */
static int pgn_cmd_add_opts(tec_pgn_cmd_t *cmd, tec_cfg_t *cfg)
{
    size_t len;
    const char *fmt = " -T %s -P %s";
    char *cmdptr = cmd->cmd + cmd->offset;

    len = snprintf(cmdptr, cmd->size, fmt, cfg->base.task, cfg->base.pgn);
    if (len >= cmd->size)
        return EXIT_FAILURE;

    cmd->offset = strlen(cmd->cmd);
    return EXIT_SUCCESS;
}

/**
 * pgn_cmd_add_args() - Append the remaining user arguments to the command buffer
 * @cmd: command buffer being built; ->offset is advanced past what was
 *       written so later appends continue from there
 * @argvec: argument vector whose entries are appended, each preceded by
 *          a space
 *
 * Return: EXIT_SUCCESS on success, EXIT_FAILURE if the buffer is too small
 */
static int pgn_cmd_add_args(tec_pgn_cmd_t *cmd, tec_argvec_t *argvec)
{
    size_t len;

    for (int i = 0; i < argvec->used; ++i) {
        char *cmdptr = cmd->cmd + cmd->offset;
        len = snprintf(cmdptr, cmd->size, " %s", argvec->argv[i]);
        if (len >= cmd->size) {
            return EXIT_FAILURE;
        }
        cmd->offset = strlen(cmd->cmd);
    }

    return EXIT_SUCCESS;
}

/**
 * pgn_cmd_genpath() - Build the full shell command line to invoke a plugin
 * @cmd: command buffer being built, filled in-place
 * @argvec: remaining user arguments to forward to the plugin
 * @cfg: active configuration
 *
 * Chains pgn_cmd_add_path(), pgn_cmd_add_opts(), and pgn_cmd_add_args() to
 * assemble "<path> -T <task> -P <pgn> <args...>" in @cmd->cmd.
 *
 * Return: 0 on success, or the value of TEC_LOG_E() naming which stage
 * overflowed the buffer
 */
static int pgn_cmd_genpath(tec_pgn_cmd_t *cmd, tec_argvec_t *argvec,
                           tec_cfg_t *cfg)
{
    if (pgn_cmd_add_path(cmd, cfg))
        return TEC_LOG_E("pgn: base generation buffer overflow");
    else if (pgn_cmd_add_opts(cmd, cfg))
        return TEC_LOG_E("pgn: option generation buffer overflow");
    else if (pgn_cmd_add_args(cmd, argvec))
        return TEC_LOG_E("pgn: argument generation buffer overflow");
    return 0;
}

/**
 * tec_cli_pgn() - Run an external plugin binary as if it were a builtin command
 * @argvec: parsed argv; argv[0] is the plugin name, the rest are forwarded
 *          to the plugin as-is
 * @cfg: active configuration, providing the plugin and task directories
 *
 * Builds the plugin's invocation command line with pgn_cmd_genpath() and
 * runs it via system().
 *
 * Return: the value of TEC_LOG_E() if the command line couldn't be built,
 * otherwise EXIT_SUCCESS if the plugin process exited with EXIT_SUCCESS,
 * else EXIT_FAILURE
 */
int tec_cli_pgn(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    tec_pgn_cmd_t cmd;
    char *name = argvec->argv[0];

    pgn_cmd_init(&cmd, name);
    argvec_offset(argvec, 1);

    if (pgn_cmd_genpath(&cmd, argvec, cfg))
        return TEC_LOG_E("pgn: command generation failed '%s'", cmd.cmd);

    TEC_LOG_D("pgn: %s", cmd.cmd);
    return system(cmd.cmd) == EXIT_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
}
