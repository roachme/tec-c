#include <string.h>

#include "tec.h"
#include "aux/toggle.h"
#include "aux/config.h"

/*
 * TODO:
 * 1. Use conts agrv. Gotta create a new dynamic struct cuz code alters it
 */

int tec_cli_cd(int argc, char **argv, tec_ctx_t *ctx)
{
    tec_arg_t args;
    const char *errfmt;
    int c, i, retcode, status;
    char alias[IDSIZ + 1] = { 0 };
    int opt_quiet, opt_help, opt_cd_dir, opt_cd_toggle;

    retcode = LIBTEC_OK;
    opt_quiet = opt_help = false;
    opt_cd_toggle = opt_cd_dir = true;
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
            opt_help = true;
            break;
        case 'n':
            opt_cd_toggle = false;
            break;
        case 'q':
            opt_quiet = true;
            break;
        case 'N':
            opt_cd_dir = false;
            opt_cd_toggle = false;
            break;
        case ':':
            return elog(1, "option `-%c' requires an argument", optopt);
        default:
            return elog(1, "invalid option `-%c'", optopt);
        }
    }

    i = optind;

    if (opt_help == true)
        return help_usage("cd");

    if ((status = check_arg_env(&args, errfmt, opt_quiet)))
        return status;
    else if ((status = check_arg_desk(&args, errfmt, opt_quiet)))
        return status;

    /* Check that alias '-' is not passed with other task IDs nor duplicated.  */
    for (int idx = i; idx < argc; ++idx) {
        if (strcmp(argv[idx], "-") == 0 && argc - i > 1)
            return elog(1, "alias '-' is used alone");
    }

    /* Resolve alias '-' to switch to previous task ID.  */
    if (argv[i] && strcmp("-", argv[i]) == 0) {
        if ((status = toggle_task_get_prev(teccfg.base.task, &args)))
            return elog(1, errfmt, "PREV", "no previous task ID");
        argv[i] = strncpy(alias, args.taskid, IDSIZ);
    }

    do {
        args.taskid = argv[i];
        if ((status = check_arg_task(&args, errfmt, opt_quiet))) {
            ;
        } else if ((status = hook_action(&args, "cd"))) {
            if (opt_quiet == false)
                elog(status, errfmt, args.taskid, "failed to execute hooks");
        } else if (opt_cd_toggle == true) {
            if ((status = toggle_task_set_curr(teccfg.base.task, &args))) {
                if (opt_quiet == false)
                    elog(1, "could not update toggles");
            }
        }
        retcode = status == LIBTEC_OK ? retcode : status;
    } while (++i < argc);

    return retcode == LIBTEC_OK && opt_cd_dir ? tec_pwd_task(&args) : retcode;
}
