#include "tec.h"
#include "aux/toggle.h"
#include "aux/config.h"

/* TODO: Find a good error message in case option fails.
 *
 *+1. Add option `-v' - explain what is being done
 * 2. Add option `-f' - ignore nonexistent files and arguments, never prompt
 * 3. Add option `-d' - remove empty/done tasks (maybe???)...
 * 4. Find when GNU rm command uses interctive mode and copy it
 * */

/* FIXME:
 * 1. When task gets deleted shell scripts tries to switch nonexistent directory
 * 2. If no task ID is passed then delete current task.
 * */
int tec_cli_rm(int argc, char **argv, tec_ctx_t *ctx)
{
    tec_arg_t args;
    const char *errfmt;
    int c, i, retcode, status;
    int opt_quiet, opt_help, opt_verbose;
    int opt_ask_once, opt_ask_every;

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
    // TODO: if non-current task gets deleted, then no need to
    // change user's current directory.
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
        } else if ((status = tec_task_del(teccfg.base.task, &args, ctx))) {
            if (opt_quiet == false)
                elog(status, errfmt, args.taskid, tec_strerror(status));
        }

        /* TODO: handle current and previos task IDs. */
        // TODO: maybe use different function to clear curr and prev task IDs?
        if (toggle_task_is_curr(teccfg.base.task, &args)) {
            toggle_task_clear(teccfg.base.task, &args, args.taskid);
        } else if (toggle_task_is_prev(teccfg.base.task, &args)) {
            toggle_task_clear(teccfg.base.task, &args, args.taskid);
        }

        if (opt_verbose == true)
            llog(0, "removed task '%s'", args.taskid);
        retcode = status == LIBTEC_OK ? retcode : status;
    } while (++i < argc);

    // FIXME: when delete task ID from non-current env,
    // it switches to current task in current env.
    // BUT should not change user's CWD at all.
    return retcode == LIBTEC_OK ? tec_pwd_task(&args) : retcode;
}
