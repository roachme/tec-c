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
    char *errfmt;
    int c, i, choice, status;
    int o_quiet, o_showhelp, o_autoconfirm, o_verbose;

    o_autoconfirm = o_quiet = o_showhelp = o_verbose = false;
    args.env = args.desk = args.taskid = NULL;
    errfmt = "cannot remove task '%s': %s";
    while ((c = getopt(argc, argv, ":b:hp:qyv")) != -1) {
        switch (c) {
        case 'b':
            args.desk = optarg;
            break;
        case 'h':
            o_showhelp = true;
            break;
        case 'p':
            args.env = optarg;
            break;
        case 'q':
            o_quiet = true;
            break;
        case 'y':
            o_autoconfirm = true;
            break;
        case 'v':
            o_verbose = true;
            break;
        case ':':
            return elog(1, "option `-%c' requires an argument", optopt);
        default:
            return elog(1, "invalid option `-%c'", optopt);
        }
    }

    if (o_showhelp == true)
        return help_usage("rm");

    if ((status = check_arg_env(&args, errfmt, o_quiet)))
        return status;
    else if ((status = check_arg_desk(&args, errfmt, o_quiet)))
        return status;

    // TODO: if non-current task gets deleted, then no need to
    // change user's current directory.
    i = optind;
    do {
        args.taskid = argv[i];

        /* Get and check input values explicitly because it's one of the rare
         * cases when hooks get exectude before the main action.  */
        if ((status = check_arg_task(&args, errfmt, o_quiet))) {
            continue;
        } else if (o_autoconfirm == false) {
            printf("Are you sure to remove task '%s'? [y/N] ", args.taskid);
            if ((choice = getchar()) != 'y' && choice != 'Y')
                continue;
        }

        if (hook_action(&args, "rm")) {
            if (o_quiet == false)
                elog(1, errfmt, args.taskid, "failed to execute hooks");
            continue;
        } else if ((status = tec_task_del(teccfg.base.task, &args, ctx))) {
            if (o_quiet == false)
                elog(status, errfmt, args.taskid, tec_strerror(status));
            continue;
        }

        /* TODO: handle current and previos task IDs. */

        if (o_verbose == true)
            llog(0, "removed task '%s'", args.taskid);
    } while (++i < argc);

    // FIXME: when delete task ID from non-current env,
    // it switches to current task in current env.
    // BUT should not change user's CWD at all.
    return status == LIBTEC_OK ? tec_pwd_task(&args) : status;
}
