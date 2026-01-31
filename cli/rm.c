#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "tec.h"
#include "aux/toggle.h"
#include "aux/config.h"

#ifdef __linux__
#include <unistd.h>

#define CWD_DELIM "/"
#endif

#ifdef __linux__
char *TEC_GETCWD(char *buf, size_t bufsiz)
{
    return getcwd(buf, bufsiz);
}
#endif

static bool do_change_user_cwd(tec_arg_t *args)
{
    char buf[FILENAME_MAX + 1] = { 0 };
    char my_user_cwd[FILENAME_MAX + 1] = { 0 };

    if (TEC_GETCWD(my_user_cwd, sizeof(my_user_cwd)) == NULL) {
        return false;
    }
    sprintf(buf, "%s/%s/%s/%s", teccfg.base.task, args->env, args->desk,
            args->taskid);

    if (strcmp(buf, my_user_cwd) == 0)
        return true;
    return false;
}

/* TODO: Find a good error message in case option fails.
 *
 * 3. Add option `-d' - remove empty/done tasks (maybe???)...
 * */

int tec_cli_rm(int argc, char **argv, tec_ctx_t *ctx)
{
    tec_arg_t args;
    const char *errfmt;
    int c, change_dir, i, retcode, status;
    int opt_quiet, opt_help, opt_verbose;
    int opt_ask_once, opt_ask_every;

    change_dir = false;
    opt_ask_every = true;       /* prompt before every removal.  */
    opt_ask_once = false;       /* prompt before once for all task IDs.  */
    retcode = LIBTEC_OK;
    errfmt = "cannot remove task '%s': %s";
    args.env = args.desk = args.taskid = NULL;
    opt_quiet = opt_help = opt_verbose = false;
    while ((c = getopt(argc, argv, ":d:e:fihqvI")) != -1) {
        switch (c) {
        case 'd':
            args.desk = optarg;
            break;
        case 'e':
            args.env = optarg;
            break;
        case 'f':
            opt_ask_every = false;
            opt_ask_once = false;
            break;
        case 'h':
            opt_help = true;
            break;
        case 'i':
            opt_ask_every = true;
            opt_ask_once = false;
            break;
        case 'q':
            opt_quiet = true;
            break;
        case 'v':
            opt_verbose = true;
            break;
        case 'I':
            opt_ask_every = false;
            opt_ask_once = true;
            break;
        case ':':
            return elog(1, "option `-%c' requires an argument", optopt);
        default:
            return elog(1, "invalid option `-%c'", optopt);
        }
    }
    i = optind;

    if (opt_help == true)
        return help_usage("rm");

    if ((status = check_arg_env(&args, errfmt, opt_quiet)))
        return status;
    else if ((status = check_arg_desk(&args, errfmt, opt_quiet)))
        return status;

    if (opt_ask_once == true) {
        printf("Are you sure to remove task(s)? [y/N] ");
        if (tec_cli_get_user_choice() == false) {
            return LIBTEC_OK;
        }
    }

    do {
        args.taskid = argv[i];

        /* Get and check input values explicitly because it's one of the rare
         * cases when hooks get exectude before the main action.  */
        if ((status = check_arg_task(&args, errfmt, opt_quiet))) {
            retcode = status == LIBTEC_OK ? retcode : status;
            continue;
        } else if (opt_ask_every == true) {
            printf("Are you sure to remove task '%s'? [y/N] ", args.taskid);
            if (tec_cli_get_user_choice() == false) {
                continue;
            }
        }

        if ((status = hook_action(&args, "rm"))) {
            if (opt_quiet == false)
                elog(1, errfmt, args.taskid, "failed to execute hooks");
            retcode = status == LIBTEC_OK ? retcode : status;
            continue;
        }
        // Compare each removed task ID with user current working directory
        if (do_change_user_cwd(&args) == true) {
            change_dir = true;
        }

        /* Update current and previos toggles.  */
        if (toggle_task_is_curr(teccfg.base.task, &args)) {
            toggle_task_unset_curr(teccfg.base.task, &args);
        } else if (toggle_task_is_prev(teccfg.base.task, &args)) {
            toggle_task_unset_curr(teccfg.base.task, &args);
        }

        if ((status = tec_task_del(teccfg.base.task, &args, ctx))) {
            if (opt_quiet == false)
                elog(status, errfmt, args.taskid, tec_strerror(status));
        }

        if (opt_verbose == true)
            llog(0, "removed task '%s'", args.taskid);

        retcode = status == LIBTEC_OK ? retcode : status;
    } while (++i < argc);

    if (change_dir) {
        args.taskid = NULL;     // ducking hotfix to get current task ID from file
        toggle_task_get_curr(teccfg.base.task, &args);
        if (args.taskid == NULL)
            args.taskid = "";
        return retcode == LIBTEC_OK ? tec_pwd_task(&args) : retcode;
    }
    return retcode;
}
