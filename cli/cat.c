/*
 * TODO:
 * 1. Add support to line-up key names in output. Make sure builtin and plugin
 *    keys line-up well.
 */

#include <stdlib.h>
#include <string.h>

#include "tec.h"
#include "aux/aux.h"
#include "aux/errno.h"
#include "aux/opts.h"
#include "aux/argvec.h"
#include "aux/config.h"

/**
 * valid_unitkeys() - Check that a unit list's keys match the expected builtin order
 * @units: linked list of task units to validate, e.g. as returned by
 *         tec_task_get()
 *
 * Walks @units alongside the global unitkeys[] array (of length nunitkey)
 * and confirms each unit's key matches the expected key at that position.
 *
 * Return: ETEC_OK if all keys line up, ETEC_UNIT_INV_KEY on the first mismatch
 */
static int valid_unitkeys(tec_unit_t *units)
{
    for (size_t i = 0; units && i < nunitkey; units = units->next, ++i)
        if (strcmp(units->key, unitkeys[i]) != 0)
            return ETEC_UNIT_INV_KEY;
    return ETEC_OK;
}

/**
 * tec_cli_cat() - Print the key/value units of one or more tasks
 * @argvec: parsed argv; remaining positional args (after options) are the
 *          task IDs to cat, processed in order
 * @cfg: active configuration
 *
 * Recognizes -d/-e (explicit desk/env), -h (help), -k KEY (repeatable;
 * restrict output to specific keys instead of all of them), -q (quiet
 * errors). For each task, validates env/desk/task, fetches its units via
 * tec_task_get(), sanity-checks their key order with valid_unitkeys(),
 * merges in any plugin-contributed units via hook_cat(), then prints
 * either every unit or just the requested -k keys.
 *
 * Return: EXIT_SUCCESS if every task and requested key resolved cleanly,
 * otherwise EXIT_FAILURE
 */
int tec_cli_cat(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    int c;
    int status;
    tec_argvec_t keys;
    int retcode = ETEC_OK;
    tec_unit_t *units = NULL;
    tec_ctx_t ctx = CTX_INIT();
    tec_unit_t *unitpgn = NULL;
    tec_arg_t args = ARGS_INIT();
    struct tec_cli_cat_options opts;

    argvec_init(&keys);
    tec_cli_cat_option_init(&opts);
    while ((c = getopt(argvec->used, argvec->argv, ":d:e:hk:q")) != -1) {
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
        case 'k':
            argvec_add(&keys, optarg);
            break;
        case 'q':
            opts.quiet = true;
            break;
        case ':':
            TEC_LOG_E(FMT_OPT_ARG_REQ, optopt);
            tec_cli_help_usage("cat");
            retcode = EXIT_FAILURE;
            goto err;
        default:
            TEC_LOG_E(FMT_OPT_ARG_INV, optopt);
            tec_cli_help_usage("cat");
            retcode = EXIT_FAILURE;
            goto err;
        }
    }
    argvec->i = optind;

    if (opts.help == true) {
        retcode = ETEC_OK;
        tec_cli_help_usage("cat");
        goto err;
    } else if ((status = tec_cli_check_env(&args, cfg))) {
        args.env = args.env ? args.env : ETEC_NOCURR;
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_TASK_CAT, args.env, tec_strerror(status));
        retcode = EXIT_FAILURE;
        goto err;
    } else if ((status = tec_cli_check_desk(&args, cfg))) {
        args.desk = args.desk ? args.desk : ETEC_NOCURR;
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_TASK_CAT, args.desk, tec_strerror(status));
        retcode = EXIT_FAILURE;
        goto err;
    }

    do {
        args.task = argvec->argv[argvec->i];

        if ((status = tec_cli_check_task(&args, cfg))) {
            retcode = EXIT_FAILURE;
            args.task = args.task ? args.task : ETEC_NOCURR;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_TASK_CAT, args.task, tec_strerror(status));
            continue;
        } else if ((status = tec_task_get(cfg->base.task, &args, &ctx))) {
            retcode = EXIT_FAILURE;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_TASK_CAT, args.task, tec_strerror(status));
            continue;
        } else if ((status = valid_unitkeys(ctx.units))) {
            retcode = EXIT_FAILURE;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_TASK_CAT, args.task, tec_strerror(status));
            continue;
        } else if ((status = hook_cat(&unitpgn, &args, "cat", cfg))) {
            retcode = EXIT_FAILURE;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_TASK_CAT, args.task, tec_strerror(status));
            continue;
        }

        units = tec_unit_add(units, "id", args.task);
        units = tec_unit_join(units, ctx.units);
        units = tec_unit_join(units, unitpgn);

        /* Show all keys.  */
        if (argvec_is_empty(&keys) == true) {
            for (tec_unit_t * tmp = units; tmp; tmp = tmp->next)
                printf(FMT_UNIT, tmp->key, tmp->val);
        } else {                /* Show specific keys only.  */
            for (int i = 0; i < keys.used; i++) {
                status = aux_show_key(keys.argv[i], units);
                if (status && opts.quiet == false)
                    TEC_LOG_E(EFMT_TASK_CAT_UNIT, args.env, keys.argv[i]);
                RETUPD(retcode, status);
            }
        }

        RETUPD(retcode, status);
        units = ctx.units = unitpgn = tec_unit_free(units);
    } while (++argvec->i < argvec->used);

 err:
    argvec_deinit(&keys);
    return retcode == ETEC_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
