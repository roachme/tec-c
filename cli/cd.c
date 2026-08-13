#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "tec.h"
#include "aux/aux.h"
#include "aux/opts.h"
#include "aux/errno.h"
#include "aux/toggle.h"
#include "aux/config.h"
#include "../lib/osdep.h"

/**
 * check_cd_path() - Validate that PATH exists inside the target task
 * @args: env/desk/task selection identifying the task directory to look inside
 * @path: PATH given to `cd -p`, checked relative to the task directory
 * @cfg: active configuration
 *
 * Return: ETEC_OK if @path names an existing directory inside the task
 * directory, otherwise ETEC_ARG_PATH_NOSUCH (including if the combined
 * path would exceed PATH_MAX)
 */
static int check_cd_path(tec_arg_t *args, const char *path, tec_cfg_t *cfg)
{
    int len;
    char pathname[PATH_MAX + 1];

    len = snprintf(pathname, sizeof(pathname), "%s/%s/%s/%s/%s",
                   cfg->base.task, args->env, args->desk, args->task, path);
    if (len < 0 || (size_t)len >= sizeof(pathname))
        return ETEC_ARG_PATH_NOSUCH;

    return ISDIR(pathname) == true ? ETEC_OK : ETEC_ARG_PATH_NOSUCH;
}

/**
 * tec_cli_cd() - Switch the "current" task to one or more given tasks
 * @argvec: parsed argv; remaining positional args (after options) are the
 *          task IDs to cd into, processed in order; "-" means the
 *          previously-current task
 * @cfg: active configuration, used to resolve current/previous toggles
 *
 * Recognizes -d/-e (explicit desk/env), -h (help), -n (don't update the
 * current-task toggle), -p PATH (switch into PATH inside the task directory
 * instead of the task directory itself), -q (quiet errors), -N (neither
 * change directory nor update the toggle). For each task argument, validates
 * env/desk/task with tec_cli_check_env()/check_desk()/check_task(), runs the
 * "cd" hook, and (unless suppressed) cascades the current-env/desk/task
 * toggles so later toggle-relative commands see the new location. Finally,
 * if -p was given, validates PATH exists inside the last successfully
 * resolved task directory via check_cd_path(), then updates the pwd file
 * to point at the resolved task (plus PATH, if given).
 *
 * Return: EXIT_SUCCESS if every task argument resolved cleanly, otherwise
 * EXIT_FAILURE (accumulated across all task arguments via RETUPD())
 */
int tec_cli_cd(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    int c;
    int status;
    int retcode = ETEC_OK;
    tec_arg_t args = ARGS_INIT();
    struct tec_cli_cd_options opts;

    tec_cli_cd_option_init(&opts);
    while ((c = getopt(argvec->used, argvec->argv, ":d:e:hnp:qN")) != -1) {
        switch (c) {
        case 'd':
            args.desk = optarg;
            break;
        case 'e':
            args.env = optarg;
            break;
        case 'h':
            opts.help = true;
            break;
        case 'n':
            opts.change_tog = false;
            break;
        case 'p':
            opts.path = optarg;
            break;
        case 'q':
            opts.quiet = true;
            break;
        case 'N':
            opts.change_dir = false;
            opts.change_tog = false;
            break;
        case ':':
            TEC_LOG_E(FMT_OPT_ARG_REQ, optopt);
            return tec_cli_help_usage("cd");
        default:
            TEC_LOG_E(FMT_OPT_ARG_INV, optopt);
            return tec_cli_help_usage("cd");
        }
    }
    argvec->i = optind;

    if (opts.help == true)
        return tec_cli_help_usage("cd");
    else if ((status = tec_cli_check_env(&args, cfg))) {
        args.env = args.env ? args.env : ETEC_NOCURR;
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_TASK_CD, args.env, tec_strerror(status));
        return EXIT_FAILURE;
    } else if ((status = tec_cli_check_desk(&args, cfg))) {
        args.desk = args.desk ? args.desk : ETEC_NOCURR;
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_TASK_CD, args.desk, tec_strerror(status));
        return EXIT_FAILURE;
    } else if ((status = tec_aux_check_cd_alias(argvec)))
        return TEC_LOG_E(tec_strerror(status));

    /* Resolve alias '-' to switch to previous task ID.  */
    if (argvec->argv[argvec->i] && strcmp("-", argvec->argv[argvec->i]) == 0) {
        if ((status = toggle_task_get_prev(cfg->base.task, &args)))
            return TEC_LOG_E(EFMT_TASK_CD, ETEC_NOPREV, tec_strerror(status));
        argvec_replace(argvec, argvec->i, args.task);
    }

    do {
        args.task = argvec->argv[argvec->i];

        if ((status = tec_cli_check_task(&args, cfg))) {
            args.task = args.task ? args.task : ETEC_NOCURR;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_TASK_CD, args.task, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        }

        if ((status = hook_action(&args, "cd", cfg))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_TASK_CD, args.task, tec_strerror(status));
        } else if (opts.change_tog == true) {
            /* Cascade so the task's env/desk become current too, not
             * just the task itself - otherwise commands that resolve
             * "current desk/env" from toggles (e.g. `tec mv`) keep
             * pointing at the old desk after cd'ing into another one. */
            status = toggle_env_set_curr(cfg->base.task, &args);
            status =
                status ? status : toggle_desk_set_curr(cfg->base.task, &args);
            status =
                status ? status : toggle_task_set_curr(cfg->base.task, &args);
            if (status && opts.quiet == false)
                TEC_LOG_E(tec_strerror(status));
        }
        RETUPD(retcode, status);
    } while (++argvec->i < argvec->used);

    if (retcode == ETEC_OK && opts.change_dir && opts.path &&
        (status = check_cd_path(&args, opts.path, cfg))) {
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_TASK_CD, opts.path, tec_strerror(status));
        retcode = status;
    } else if (retcode == ETEC_OK && opts.change_dir) {
        retcode = opts.path ? tec_cli_pwd_set_path(&args, opts.path, cfg)
            : tec_cli_pwd_set(&args, cfg);
    }
    return retcode == ETEC_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
