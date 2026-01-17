#include <string.h>

#include "tec.h"
#include "aux/toggle.h"
#include "aux/config.h"

int tec_cli_cd(int argc, char **argv, tec_ctx_t *ctx)
{
    tec_arg_t args;
    char c, *errfmt;
    int i, quiet, showhelp, status;
    int switch_toggle, switch_dir, swap_toggle;

    quiet = showhelp = false;
    switch_toggle = switch_dir = true;
    swap_toggle = false;
    errfmt = "cannot switch to '%s': %s";
    args.env = args.board = args.taskid = NULL;
    while ((c = getopt(argc, argv, ":b:hnp:qN")) != -1) {
        switch (c) {
        case 'b':
            args.board = optarg;
            break;
        case 'h':
            showhelp = true;
            break;
        case 'n':
            switch_toggle = false;
            break;
        case 'p':
            args.env = optarg;
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
    else if ((status = check_arg_board(&args, errfmt, quiet)))
        return status;

    i = optind;

    /* Alias to switch to previous task ID.  */
    if (argv[i] && strcmp("-", argv[i]) == 0) {
        argv[i] = NULL;         /* NULL it cuz it's an alias and illegal task ID.  */
        swap_toggle = true;
        if ((status = toggle_task_get_prev(teccfg.base.task, &args)))
            return elog(1, errfmt, "PREV", "could not get previous task ID");
    }

    do {
        args.taskid = args.taskid != NULL ? args.taskid : argv[i];

        if ((status = check_arg_task(&args, errfmt, quiet)))
            continue;
        else if (hook_action(&args, "cd")) {
            if (quiet == false)
                elog(status, errfmt, args.taskid, "failed to execute hooks");
            continue;
        }
    } while (++i < argc);

    if (status == LIBTEC_OK && switch_toggle == true) {
        if (swap_toggle) {
            if (toggle_task_swap(teccfg.base.task, &args) && quiet == false)
                status = elog(1, "could not update toggles");
        } else {
            if (toggle_task_set_curr(teccfg.base.task, &args) && quiet == false)
                status = elog(1, "could not update toggles");
        }
    }

    return status == LIBTEC_OK && switch_dir ? tec_pwd_task(&args) : status;
}
