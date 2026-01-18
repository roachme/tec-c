#include <string.h>

#include "tec.h"
#include "aux/toggle.h"
#include "aux/config.h"

int tec_cli_cd(int argc, char **argv, tec_ctx_t *ctx)
{
    tec_arg_t args;
    char c, *errfmt;
    int i, quiet, showhelp, status;
    int switch_toggle, switch_dir;
    char *new_curr, *new_prev;

    quiet = showhelp = false;
    new_curr = new_prev = NULL;
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

    /* Alias to switch to previous task ID.  */
    if (argv[i] && strcmp("-", argv[i]) == 0) {
        argv[i] = NULL;         /* NULL it cuz it's an alias and illegal task ID.  */
        if ((status = toggle_task_get_prev(teccfg.base.task, &args)))
            return elog(1, errfmt, "PREV", "could not get previous task ID");
    }

    do {
        args.taskid = args.taskid != NULL ? args.taskid : argv[i];
        new_prev = new_curr;
        new_curr = args.taskid;

        printf("-- %s\n", args.taskid);
        if ((status = check_arg_task(&args, errfmt, quiet))) {
            args.taskid = NULL;     /* unset task ID, not to break loop.  */
            continue;
        }
        else if (hook_action(&args, "cd")) {
            if (quiet == false)
                elog(status, errfmt, args.taskid, "failed to execute hooks");
            args.taskid = NULL;     /* unset task ID, not to break loop.  */
            continue;
        }

        args.taskid = NULL;     /* unset task ID, not to break loop.  */
    } while (++i < argc);

    if (status == LIBTEC_OK && switch_toggle == true) {
        if (new_prev) {
            printf("update new prev '%s'\n", new_prev);
            args.taskid = new_prev;
            if (toggle_task_set_curr(teccfg.base.task, &args) && quiet == false)
                status = elog(1, "could not update toggles");
        }
        if (new_curr) {
            printf("update new curr '%s'\n", new_curr);
            args.taskid = new_curr;
            if (toggle_task_set_curr(teccfg.base.task, &args) && quiet == false)
                status = elog(1, "could not update toggles");
        }
    }

    return status == LIBTEC_OK && switch_dir ? tec_pwd_task(&args) : status;
}
