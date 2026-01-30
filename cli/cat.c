#include <string.h>

#include "tec.h"
#include "aux/toggle.h"
#include "aux/config.h"

// TODO: add support to show multiple key values

static const char *errfmt = "cannot show units '%s': %s";

static int valid_unitkeys(tec_unit_t *units)
{
    for (int i = 0; units && i < nunitkey; units = units->next) {
        if (strcmp(units->key, unitkeys[i]) != 0)
            return 1;
        ++i;
    }
    return 0;
}

static int show_key(char *task, tec_unit_t *unitbin, tec_unit_t *unitpgn,
                    char *key)
{
    struct tec_unit *units;

    if (strcmp(key, "id") == 0) {
        printf("%s\n", task);
        return 0;
    }

    for (units = unitbin; units; units = units->next)
        if (strcmp(key, units->key) == 0) {
            printf("%s\n", units->val);
            return 0;
        }

    for (; unitpgn; unitpgn = unitpgn->next)
        if (strcmp(key, unitpgn->key) == 0) {
            printf("%s\n", unitpgn->val);
            return 0;
        }

    return 1;
}

static int show_keys(char *task, tec_unit_t *unitbin, tec_unit_t *unitpgn)
{
    const char *fmt = "%-" xstr(PADDING_UNIT) "s : %s\n";

    printf(fmt, "id", task);

    for (struct tec_unit * units = unitbin; units; units = units->next)
        printf(fmt, units->key, units->val);

    for (; unitpgn; unitpgn = unitpgn->next)
        printf(fmt, unitpgn->key, unitpgn->val);

    return 0;
}

int tec_cli_cat(int argc, char **argv, tec_ctx_t *ctx)
{
    char *key;
    tec_arg_t args;
    tec_unit_t *unitpgn;
    int opt_quiet, opt_help;
    int c, i, retcode, status;

    key = NULL;
    unitpgn = NULL;
    retcode = LIBTEC_OK;
    opt_quiet = opt_help = false;
    args.env = args.desk = args.taskid = NULL;
    while ((c = getopt(argc, argv, ":d:e:hk:q")) != -1) {
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
        case 'k':
            key = optarg;
            break;
        case 'q':
            opt_quiet = true;
            break;
        case ':':
            return elog(1, "option `-%c' requires an argument", optopt);
        default:
            return elog(1, "invalid option `-%c'", optopt);
        }
    }
    i = optind;

    if (opt_help == true)
        return help_usage("cat");

    if ((status = check_arg_env(&args, errfmt, opt_quiet)))
        return status;
    else if ((status = check_arg_desk(&args, errfmt, opt_quiet)))
        return status;

    do {
        args.taskid = argv[i];

        if ((status = check_arg_task(&args, errfmt, opt_quiet))) {
            ;
        } else if ((status = tec_task_get(teccfg.base.task, &args, ctx))) {
            if (opt_quiet == false)
                elog(status, errfmt, args.taskid, tec_strerror(status));
        } else if ((status = valid_unitkeys(ctx->units))) {
            if (opt_quiet == false)
                elog(status, errfmt, args.taskid, "invalid unit keys");
        } else if ((status = hook_show(&unitpgn, &args, "cat"))) {
            if (opt_quiet == false)
                elog(status, errfmt, args.taskid, "failed to execute hooks");
        } else if (key != NULL) {
            if ((status = show_key(args.taskid, ctx->units, unitpgn, key)))
                if (opt_quiet == false)
                    elog(1, "cannot show key '%s': no such key", key);
        } else if ((status = show_keys(args.taskid, ctx->units, unitpgn))) {
            if (opt_quiet == false)
                elog(1, errfmt, args.taskid, "internal error");
        }

        unitpgn = tec_unit_free(unitpgn);
        ctx->units = tec_unit_free(ctx->units);
        retcode = status == LIBTEC_OK ? retcode : status;
    }
    while (++i < argc);
    return retcode;
}
