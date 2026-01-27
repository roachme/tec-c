#include <string.h>

#include "tec.h"
#include "aux/toggle.h"
#include "aux/config.h"

int tec_cli_cd(int argc, char **argv, tec_ctx_t *ctx)
{
    tec_arg_t args;
    char c, *errfmt;
    char alias[IDSIZ + 1] = { 0 };
    int i, quiet, showhelp, retcode, status;
    int switch_toggle, switch_dir;

    retcode = LIBTEC_OK;
    quiet = showhelp = false;
    switch_toggle = switch_dir = true;
    errfmt = "cannot switch to '%s': %s";
    args.env = args.desk = args.taskid = NULL;
    while ((c = getopt(argc, argv, ":d:e:hnqN")) != -1) {
        switch (c) {
        case 'd':
            args.desk = optarg;
            break;
        case 'e':
            args.env = optarg;
            break;
        case 'h':
            showhelp = true;
            break;
        case 'n':
            switch_toggle = false;
            break;
        case 'q':
            quiet = true;
            break;
        case 'N':
            switch_dir = false;
            switch_toggle = false;
            break;
        case ':':
            return elog(1, "option `-%c' requires an argument", optopt);
        default:
            return elog(1, "invalid option `-%c'", optopt);
        }
    }

    if (showhelp == true)
        return help_usage("cd");

    if ((status = check_arg_env(&args, errfmt, quiet)))
        return status;
    else if ((status = check_arg_desk(&args, errfmt, quiet)))
        return status;

    i = optind;

    /* Check that alias '-' is not passed among task IDs.  */
    for (int idx = 1; idx < argc; ++idx) {
        if (strcmp(argv[idx], "-") == 0 && argc > 2)
            return elog(1, "alias '-' cannot be used with other task IDs");
    }

    do {
        args.taskid = argv[i];
        retcode = status == LIBTEC_OK ? retcode : status;

        /* TODO: move alias logic out of loop. But before that create custom
         * structure to store argv and argc cuz they're gonno be rewritten.  */

        /* Alias to switch to previous task ID.  */
        if (args.taskid && strcmp("-", args.taskid) == 0) {
            args.taskid = NULL; /* unset task ID.  */
            if ((status = toggle_task_get_prev(teccfg.base.task, &args)))
                return elog(1, errfmt, "PREV", "no previous task ID");
            args.taskid = strncpy(alias, args.taskid, IDSIZ);
        }

        if ((status = check_arg_task(&args, errfmt, quiet))) {
            continue;
        } else if (hook_action(&args, "cd")) {
            if (quiet == false)
                elog(status, errfmt, args.taskid, "failed to execute hooks");
            status = 1;         /* TODO: use cli return codes.  */
            continue;
        } else if (switch_toggle == true) {
            if (toggle_task_set_curr(teccfg.base.task, &args) && quiet == false) {
                if (quiet == false)
                    elog(1, "could not update toggles");
                status = 1;     /* TODO: use cli return codes.  */
                continue;
            }
        }
    } while (++i < argc);

    retcode = status == LIBTEC_OK ? retcode : status;
    return retcode == LIBTEC_OK && switch_dir ? tec_pwd_task(&args) : retcode;
}
