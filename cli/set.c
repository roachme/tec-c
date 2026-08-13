#include <stdlib.h>

#include "tec.h"
#include "aux/aux.h"
#include "aux/opts.h"
#include "aux/errno.h"
#include "aux/config.h"

// "prio",  /* task priority */
// "type",  /* task type: bugfix, hotfix, feature */
// "date",  /* task date of creation */
// "desc",  /* task description */
// "user"   /* who created, who's woring on it */
// "users"  /* list of users */
// "teams"  /* list of teams */
// "label"  /* list of labels */
// "time"   /* time tracker */

/**
 * tec_cli_set() - Set one or more unit values on one or more tasks
 * @argvec: parsed argv; remaining positional args (after options) are the
 *          task IDs to update, processed in order
 * @cfg: active configuration
 *
 * Recognizes -d/-e (explicit desk/env), -h (help), -q (quiet errors), -i
 * (interactive; currently unimplemented, always returns TEC_LOG_E(EFMT_DEV)),
 * -T TYPE / -D DESC / -P PRIO to stage the "type"/"desc"/"prio" units
 * (each validated with tec_aux_is_valid_type()/_desc()/_prio() and only
 * staged once, first occurrence wins). The staged units are applied to
 * every task argument via tec_task_set(), followed by the "set" hook.
 *
 * Return: EXIT_SUCCESS if every task and staged unit applied cleanly,
 * otherwise EXIT_FAILURE; or the value of TEC_LOG_E(EFMT_DEV) if -i was given
 */
int tec_cli_set(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    int c;
    int status = ETEC_OK;
    int retcode = ETEC_OK;
    tec_ctx_t ctx = CTX_INIT();
    tec_arg_t args = ARGS_INIT();
    struct tec_cli_set_options opts;

    tec_cli_set_option_init(&opts);
    while ((c = getopt(argvec->used, argvec->argv, ":d:e:hiqD:T:P:")) != -1) {
        switch (c) {
        case 'd':
            args.desk = optarg;
            break;
        case 'e':
            args.env = optarg;
            break;
        case 'q':
            opts.quiet = true;
            break;
        case 'h':
            opts.help = true;
            break;
        case 'i':
            return TEC_LOG_E(EFMT_DEV, c);
        case 'T':
            if (tec_aux_is_valid_type(optarg) == false) {
                TEC_LOG_E("invalid type '%s'", optarg);
                return tec_cli_help_usage("set");
            }
            if (tec_unit_get(ctx.units, "type") == NULL)
                ctx.units = tec_unit_add(ctx.units, "type", optarg);
            break;
        case 'D':
            if (tec_aux_is_valid_desc(optarg) == false) {
                TEC_LOG_E("invalid description '%s'", optarg);
                return tec_cli_help_usage("set");
            }
            if (tec_unit_get(ctx.units, "desc") == NULL)
                ctx.units = tec_unit_add(ctx.units, "desc", optarg);
            break;
        case 'P':
            if (tec_aux_is_valid_prio(optarg) == false) {
                TEC_LOG_E("invalid priority '%s'", optarg);
                return tec_cli_help_usage("set");
            }
            if (tec_unit_get(ctx.units, "prio") == NULL)
                ctx.units = tec_unit_add(ctx.units, "prio", optarg);
            break;
        case ':':
            TEC_LOG_E(FMT_OPT_ARG_REQ, optopt);
            return tec_cli_help_usage("set");
        default:
            TEC_LOG_E(FMT_OPT_ARG_INV, optopt);
            return tec_cli_help_usage("set");
        }
    }
    argvec->i = optind;

    if (opts.help == true)
        return tec_cli_help_usage("set");
    else if ((status = tec_cli_check_env(&args, cfg))) {
        args.env = args.env ? args.env : ETEC_NOCURR;
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_TASK_SET, args.env, tec_strerror(status));
        return EXIT_FAILURE;
    } else if ((status = tec_cli_check_desk(&args, cfg))) {
        args.desk = args.desk ? args.desk : ETEC_NOCURR;
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_TASK_SET, args.desk, tec_strerror(status));
        return EXIT_FAILURE;
    }

    do {
        args.task = argvec->argv[argvec->i];

        if ((status = tec_cli_check_task(&args, cfg))) {
            args.task = args.task ? args.task : ETEC_NOCURR;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_TASK_SET, args.task, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        }

        if ((status = tec_task_set(cfg->base.task, &args, &ctx))) {
            args.task = args.task ? args.task : "NOCURR";
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_TASK_SET, args.task, tec_strerror(status));
        } else if ((status = hook_action(&args, "set", cfg))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_TASK_SET, args.task, tec_strerror(status));
        }

        RETUPD(retcode, status);
    } while (++argvec->i < argvec->used);

    ctx.units = tec_unit_free(ctx.units);
    return retcode == ETEC_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
