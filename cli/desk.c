#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "tec.h"
#include "aux/opts.h"
#include "../lib/list.h"
#include "aux/errno.h"
#include "aux/toggle.h"
#include "aux/config.h"

// TODO: unify it (env, desk, task)
/**
 * valid_unitkeys() - Check that a desk's unit list only contains "desc"
 * @units: linked list of desk units to validate, e.g. as returned by
 *         tec_desk_get()
 *
 * Return: ETEC_OK if the keys match the expected order, ETEC_UNIT_INV_KEY
 * on the first mismatch
 */
static int valid_unitkeys(tec_unit_t *units)
{
    const char *keys[] = { "desc" };
    unsigned int nkeys = ARRAY_SIZE(keys);

    for (size_t i = 0; units && i < nkeys; units = units->next, ++i)
        if (strcmp(units->key, keys[i]) != 0)
            return ETEC_UNIT_INV_KEY;
    return ETEC_OK;
}

// TODO: remove parameter 'quiet', return status, and let the caller to
/**
 * get_unit_desc() - Fetch a desk's "desc" unit value
 * @ctx: filled in by tec_desk_get() with the desk's units on success
 * @args: identifies the desk to fetch (args->desk must be set)
 * @quiet: suppress TEC_LOG_E() output when set
 * @cfg: active configuration
 *
 * Return: the desk's description string, or NULL if the desk couldn't be
 * fetched or has no "desc" unit
 */
static char *get_unit_desc(tec_ctx_t *ctx, tec_arg_t *args, int quiet,
                           tec_cfg_t *cfg)
{
    int status;
    char *desc = NULL;

    if ((status = tec_desk_get(cfg->base.task, args, ctx))) {
        if (quiet == false)
            TEC_LOG_E("'%s': %s one", args->desk, tec_strerror(status));
    } else if ((desc = tec_unit_get(ctx->units, "desc")) == NULL) {
        if (quiet == false)
            TEC_LOG_E("'%s': %s", args->desk, "description not found");
    }
    return desc;
}

/**
 * generate_units() - Build the default unit set for a newly added desk
 * @ctx: ->units is set to the newly-built unit list (just "desc")
 * @desk: desk name used in the auto-generated description
 * @desc: user-supplied -D description, or NULL to auto-generate one
 *        ("Generated desciption for <desk>")
 *
 * Return: 0 on success, 1 if the resulting unit list is empty
 */
static int generate_units(tec_ctx_t *ctx, char *desk, char *desc)
{
    struct tec_unit *units = NULL;
    char _desc[100] = "Generated desciption for ";

    /* Set custom description if provided.  */
    if (desc == NULL) {         /* Generate description.  */
        strcat(_desc, desk);
        desc = _desc;
    }

    units = tec_unit_add(units, "desc", desc);
    if ((ctx->units = units) == NULL)
        return 1;
    return 0;
}

// TODO: add support to generate desk name
/**
 * _desk_add() - Implement `tec desk add`, creating one or more desks
 * @argvec: parsed argv (subcommand name already skipped); remaining
 *          positional args are the desk names to add, processed in order
 * @cfg: active configuration
 *
 * Recognizes -e (explicit env), -h (help), -n (don't update the
 * current-desk toggle), -q (quiet errors), -D DESC (custom description),
 * -N (neither change directory nor update the toggle). Unlike `tec add`,
 * a desk name is required (no auto-generation). For each desk name:
 * validates its length and format (tec_cli_len_valid()/tec_desk_valid()),
 * rejects it if it already exists, builds its default units
 * (generate_units()), creates it via tec_desk_add(), runs the
 * "desk-add" hook, and (unless suppressed) sets it as the current desk.
 *
 * Return: EXIT_SUCCESS if every desk was added cleanly, otherwise EXIT_FAILURE
 */
static int _desk_add(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    char c;
    int status;
    char *desc = NULL;
    int retcode = ETEC_OK;
    tec_ctx_t ctx = CTX_INIT();
    tec_arg_t args = ARGS_INIT();
    struct tec_cli_cd_options opts;

    tec_cli_cd_option_init(&opts);
    while ((c = getopt(argvec->used, argvec->argv, ":e:hnqD:N")) != -1) {
        switch (c) {
        case 'e':
            args.env = optarg;
            break;
        case 'h':
            opts.help = true;
            break;
        case 'n':
            opts.change_tog = false;
            break;
        case 'q':
            opts.quiet = true;
            break;
        case 'D':
            desc = optarg;
            break;
        case 'N':
            opts.change_dir = false;
            opts.change_tog = false;
            break;
        case ':':
            TEC_LOG_E(FMT_OPT_ARG_REQ, optopt);
            return tec_cli_help_usage("desk-add");
        default:
            TEC_LOG_E(FMT_OPT_ARG_INV, optopt);
            return tec_cli_help_usage("desk-add");
        }
    }
    argvec->i = optind;

    if (opts.help == true)
        return tec_cli_help_usage("desk-add");
    else if (optind == argvec->used)
        return TEC_LOG_E(tec_strerror(ETEC_ARG_DESK_REQ));
    else if ((status = tec_cli_check_env(&args, cfg))) {
        args.env = args.env ? args.env : ETEC_NOCURR;
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_DESK_ADD, args.env, tec_strerror(status));
        return EXIT_FAILURE;
    }

    do {
        args.desk = argvec->argv[argvec->i];

        if (tec_cli_len_valid(args.desk, DESKSIZ) == false) {
            status = ETEC_ARG_DESK_TOO_LONG;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_ADD, args.desk, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        } else if ((status = tec_desk_valid(cfg->base.task, &args))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_ADD, args.desk, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        } else if (!(status = tec_desk_exist(cfg->base.task, &args))) {
            status = ETEC_ARG_DESK_EXIST;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_ADD, args.desk, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        } else if ((status = generate_units(&ctx, args.desk, desc))) {
            status = ETEC_UNIT_GEN_FAIL;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_ADD, args.desk, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        }

        if ((status = tec_desk_add(cfg->base.task, &args, &ctx))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_ADD, args.desk, tec_strerror(status));
            ctx.units = tec_unit_free(ctx.units);
        } else if ((status = hook_action(&args, "desk-add", cfg))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_ADD, args.task, tec_strerror(status));
        } else if (opts.change_tog == true) {
            if ((status = toggle_desk_set_curr(cfg->base.task, &args))) {
                if (opts.quiet == false)
                    TEC_LOG_E(tec_strerror(status));
            }
        }
        ctx.units = tec_unit_free(ctx.units);
        retcode = status == ETEC_OK ? EXIT_SUCCESS : EXIT_FAILURE;
    } while (++argvec->i < argvec->used);

    if (retcode == ETEC_OK && opts.change_dir) {
        status = tec_cli_pwd_set(&args, cfg);
        RETUPD(retcode, status);
    }
    return retcode == ETEC_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**
 * _desk_rm() - Implement `tec desk rm`, removing one or more desks
 * @argvec: parsed argv (subcommand name already skipped); remaining
 *          positional args are the desk names to remove, processed in order
 * @cfg: active configuration
 *
 * Recognizes -f (never prompt, RMI_NEVER), -h (help), -i (always prompt
 * per desk, RMI_ALWAYS), -q (quiet errors), -v (verbose, log each
 * removal), -I (prompt once up front when removing more than 3 desks,
 * RMI_SOMETIMES; the default mode). For each desk: runs the "desk-rm"
 * hook then removes it via tec_desk_rm(). Afterwards refreshes the pwd
 * file if needed.
 *
 * Return: EXIT_SUCCESS if every desk was removed (or skipped by the user
 * at a prompt) cleanly, otherwise EXIT_FAILURE
 */
static int _desk_rm(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    int c;
    int status;
    int ntasks;
    int retcode = ETEC_OK;
    tec_ctx_t ctx = CTX_INIT();
    tec_arg_t args = ARGS_INIT();
    struct tec_cli_rm_options opts;

    tec_cli_rm_option_init(&opts);
    while ((c = getopt(argvec->used, argvec->argv, ":e:fhiqvI")) != -1) {
        switch (c) {
        case 'f':
            opts.mode = RMI_NEVER;
            break;
        case 'h':
            opts.help = true;
            break;
        case 'i':
            opts.mode = RMI_ALWAYS;
            break;
        case 'q':
            opts.quiet = true;
            break;
        case 'v':
            opts.verbose = true;
            break;
        case 'I':
            opts.mode = RMI_SOMETIMES;
            break;
        case ':':
            TEC_LOG_E(FMT_OPT_ARG_REQ, optopt);
            return tec_cli_help_usage("desk-rm");
        default:
            TEC_LOG_E(FMT_OPT_ARG_INV, optopt);
            return tec_cli_help_usage("desk-rm");
        }
    }
    argvec->i = optind;
    ntasks = argvec->used - argvec->i;

    if (opts.help)
        return tec_cli_help_usage("desk-rm");
    else if ((status = tec_cli_check_env(&args, cfg))) {
        args.env = args.env ? args.env : ETEC_NOCURR;
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_DESK_RM, args.env, tec_strerror(status));
        return EXIT_FAILURE;
    }

    if (ntasks > 3 && opts.mode == RMI_SOMETIMES) {
        TEC_LOG_P("remove %d desks? [y/N] ", ntasks);
        if (!tec_aux_yesno()) {
            return EXIT_SUCCESS;
        }
    }

    do {
        args.desk = argvec->argv[argvec->i];

        if ((status = tec_cli_check_desk(&args, cfg))) {
            args.desk = args.desk ? args.desk : ETEC_NOCURR;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_RM, args.desk, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        } else if (opts.mode == RMI_ALWAYS) {
            TEC_LOG_P("remove desk '%s'? [y/N] ", args.task);
            if (!tec_aux_yesno())
                continue;
        }
        // TODO: update current directory if current env got deleted.
        if ((status = hook_action(&args, "desk-rm", cfg))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_RM, args.desk, tec_strerror(status));
        } else if ((status = tec_desk_rm(cfg->base.task, &args, &ctx))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_RM, args.desk, tec_strerror(status));
        }

        if (status == ETEC_OK && opts.verbose == true)
            TEC_LOG_I("removed desk '%s'", args.task);
        RETUPD(retcode, status);
    } while (++argvec->i < argvec->used);

    if (retcode == ETEC_OK && opts.change_dir) {
        args.desk = NULL;       /* Force to get current task ID.  */
        status = tec_cli_pwd_set(&args, cfg);
        if (toggle_desk_get_curr(cfg->base.task, &args))
            args.desk = "";
        RETUPD(retcode, status);
    }
    return retcode == ETEC_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**
 * cmp_desk_id() - qsort() comparator ordering tec_list_t entries by desk name
 * @a: pointer to a tec_list_t element
 * @b: pointer to a tec_list_t element
 *
 * Return: result of strcmp() on the two entries' ->name fields
 */
static int cmp_desk_id(const void *a, const void *b)
{
    const tec_list_t *ea = a;
    const tec_list_t *eb = b;

    return strcmp(ea->name, eb->name);
}

/**
 * _desk_ls() - Implement `tec desk ls`, listing the desks in one or more environments
 * @argvec: parsed argv (subcommand name already skipped); remaining
 *          positional args are the environment names to list, processed
 *          in order
 * @cfg: active configuration
 *
 * Recognizes -e (explicit env, though env is also read positionally per
 * iteration), -h (help), -q (quiet errors). For each environment,
 * fetches the desk list via tec_desk_list(), sorts it by name
 * (cmp_desk_id()), and prints each desk's ID and description.
 *
 * Return: the status of the last processed environment (does not
 * accumulate failures across multiple environment arguments)
 */
static int _desk_ls(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    char *desc = NULL;
    int c, i, status;
    tec_ctx_t ctx = CTX_INIT();
    tec_arg_t args = ARGS_INIT();
    int opt_help, opt_quiet;

    opt_help = opt_quiet = false;

    while ((c = getopt(argvec->used, argvec->argv, ":e:hq")) != -1) {
        switch (c) {
        case 'q':
            opt_quiet = true;
            break;
        case 'h':
            opt_help = true;
            break;
        case ':':
            TEC_LOG_E(FMT_OPT_ARG_REQ, optopt);
            return tec_cli_help_usage("desk-ls");
        default:
            TEC_LOG_E(FMT_OPT_ARG_INV, optopt);
            return tec_cli_help_usage("desk-ls");
        }
    }

    if (opt_help == true)
        return tec_cli_help_usage("desk-ls");
    else if ((status = tec_cli_check_env(&args, cfg))) {
        args.env = args.env ? args.env : ETEC_NOCURR;
        if (opt_quiet == false)
            TEC_LOG_E(EFMT_DESK_LS, args.env, tec_strerror(status));
        return EXIT_FAILURE;
    }

    i = optind;
    do {
        args.env = argvec->argv[i];

        // TODO: why check env twice? it was done above
        if ((status = tec_cli_check_env(&args, cfg))) {
            args.env = args.env ? args.env : ETEC_NOCURR;
            if (opt_quiet == false)
                TEC_LOG_E(EFMT_DESK_LS, args.env, tec_strerror(status));
            continue;
        }
        if ((status = tec_desk_list(cfg->base.task, &args, &ctx))) {
            if (opt_quiet == false)
                TEC_LOG_E(EFMT_DESK_LS, args.desk, tec_strerror(status));
            continue;
        }

        if (list_size(ctx.list) > 0)
            qsort(ctx.list->items, ctx.list->count, sizeof(*ctx.list->items),
                  cmp_desk_id);

        for (size_t idx = 0; idx < list_size(ctx.list); idx++) {
            tec_list_t *obj = &ctx.list->items[idx];

            args.desk = obj->name;
            if ((desc = get_unit_desc(&ctx, &args, opt_quiet, cfg)) == NULL) {
                continue;
            }
            LIST_OBJ_UNITS(obj->name, "", desc, DESKSIZ, cfg->opts.color);
            ctx.units = tec_unit_free(ctx.units);
        }
        ctx.list = tec_list_free(ctx.list);
    } while (++i < argvec->used);

    // FIXME: unify return code logic with the rest of the commands
    return status;
}

/**
 * _desk_mv() - Implement `tec desk mv` (currently unimplemented)
 * @argvec: unused
 * @cfg: unused
 *
 * Return: the value of TEC_LOG_E(), always
 */
static int _desk_mv(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    (void)argvec;
    (void)cfg;
    return TEC_LOG_E("%s: under development", __func__);
}

/**
 * _desk_set() - Implement `tec desk set`, setting the "desc" unit on one or more desks
 * @argvec: parsed argv (subcommand name already skipped); remaining
 *          positional args are the desk names to update, processed in order
 * @cfg: active configuration
 *
 * Recognizes -e (explicit env), -h (help), -q (quiet errors), -D DESC
 * (staged "desc" unit, validated with tec_aux_is_valid_desc()). The
 * staged unit is applied to every desk via tec_desk_set(), followed by
 * the "desk-set" hook.
 *
 * Return: EXIT_SUCCESS if every desk updated cleanly, otherwise EXIT_FAILURE
 */
static int _desk_set(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    int c;
    int status;
    int retcode = ETEC_OK;
    tec_ctx_t ctx = CTX_INIT();
    tec_arg_t args = ARGS_INIT();
    struct tec_cli_set_options opts;

    tec_cli_set_option_init(&opts);
    while ((c = getopt(argvec->used, argvec->argv, ":e:hqD:")) != -1) {
        switch (c) {
        case 'h':
            opts.help = true;
            break;
        case 'q':
            opts.quiet = true;
            break;
        case 'D':
            if (tec_aux_is_valid_desc(optarg) == false) {
                TEC_LOG_E("invalid description '%s'", optarg);
                return tec_cli_help_usage("desk-set");
            }
            ctx.units = tec_unit_add(ctx.units, "desc", optarg);
            break;
        case ':':
            TEC_LOG_E(FMT_OPT_ARG_REQ, optopt);
            return tec_cli_help_usage("desk-set");
        default:
            TEC_LOG_E(FMT_OPT_ARG_INV, optopt);
            return tec_cli_help_usage("desk-set");
        }
    }
    argvec->i = optind;

    if (opts.help)
        return tec_cli_help_usage("desk-set");
    else if ((status = tec_cli_check_env(&args, cfg))) {
        args.env = args.env ? args.env : ETEC_NOCURR;
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_DESK_SET, args.env, tec_strerror(status));
        return EXIT_FAILURE;
    }

    do {
        args.desk = argvec->argv[argvec->i];

        if ((status = tec_cli_check_desk(&args, cfg))) {
            args.desk = args.desk ? args.desk : ETEC_NOCURR;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_SET, args.desk, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        }

        if ((status = tec_desk_set(cfg->base.task, &args, &ctx))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_SET, args.desk, tec_strerror(status));
        } else if ((status = hook_action(&args, "desk-set", cfg))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_SET, args.task, tec_strerror(status));
        }

        RETUPD(retcode, status);
    } while (++argvec->i < argvec->used);

    ctx.units = tec_unit_free(ctx.units);
    return retcode == ETEC_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**
 * _desk_cat() - Implement `tec desk cat`, printing the units of one or more desks
 * @argvec: parsed argv (subcommand name already skipped); remaining
 *          positional args are the desk names to cat, processed in order
 * @cfg: active configuration
 *
 * Recognizes -e (explicit env), -h (help), -k KEY (repeatable; restrict
 * output to specific keys), -q (quiet errors). For each desk, validates
 * it, sanity-checks its unit key order with valid_unitkeys(), fetches its
 * units via tec_desk_get(), merges in any plugin-contributed units via
 * hook_cat(), then prints either every unit or just the requested -k keys.
 *
 * Return: EXIT_SUCCESS if every desk and requested key resolved cleanly,
 * otherwise EXIT_FAILURE
 */
static int _desk_cat(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    int c;
    int status;
    tec_argvec_t keys;
    int retcode = ETEC_OK;
    tec_ctx_t ctx = CTX_INIT();
    tec_arg_t args = ARGS_INIT();
    struct tec_unit *units = NULL;
    struct tec_unit *unitpgn = NULL;
    struct tec_cli_cd_options opts;

    argvec_init(&keys);
    tec_cli_cd_option_init(&opts);
    while ((c = getopt(argvec->used, argvec->argv, ":e:hk:q")) != -1) {
        switch (c) {
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
            tec_cli_help_usage("desk-cat");
            retcode = EXIT_FAILURE;
            goto err;
        default:
            TEC_LOG_E(FMT_OPT_ARG_INV, optopt);
            tec_cli_help_usage("desk-cat");
            retcode = EXIT_FAILURE;
            goto err;
        }
    }
    argvec->i = optind;

    if (opts.help) {
        retcode = ETEC_OK;
        tec_cli_help_usage("desk-cat");
        goto err;
    } else if ((status = tec_cli_check_env(&args, cfg))) {
        args.env = args.env ? args.env : ETEC_NOCURR;
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_DESK_CAT, args.env, tec_strerror(status));
        retcode = EXIT_FAILURE;
        goto err;
    }

    do {
        args.desk = argvec->argv[argvec->i];

        if ((status = tec_cli_check_desk(&args, cfg))) {
            args.desk = args.desk ? args.desk : ETEC_NOCURR;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_CAT, args.desk, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        } else if ((status = valid_unitkeys(ctx.units))) {
            retcode = EXIT_FAILURE;
            status = ETEC_UNIT_INV_KEY;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_CAT, args.desk, tec_strerror(status));
            continue;
        } else if ((status = tec_desk_get(cfg->base.task, &args, &ctx))) {
            retcode = EXIT_FAILURE;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_CAT, args.desk, tec_strerror(status));
            continue;
        } else if ((status = hook_cat(&unitpgn, &args, "desk-cat", cfg))) {
            retcode = EXIT_FAILURE;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_CAT, args.desk, tec_strerror(status));
            continue;
        }

        units = tec_unit_add(units, "id", args.desk);
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
                    TEC_LOG_E(EFMT_DESK_CAT_UNIT, args.env, keys.argv[i]);
                retcode = status == EXIT_SUCCESS ? retcode : EXIT_FAILURE;
            }
        }

        units = ctx.units = unitpgn = tec_unit_free(units);
        RETUPD(retcode, status);
    } while (++argvec->i < argvec->used);

 err:
    argvec_deinit(&keys);
    return retcode == ETEC_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**
 * _desk_cd() - Implement `tec desk cd`, switching the "current" desk
 * @argvec: parsed argv (subcommand name already skipped); remaining
 *          positional args are the desk names to cd into, processed in
 *          order; "-" means the previously-current desk
 * @cfg: active configuration
 *
 * Recognizes -e (explicit env), -h (help), -n (don't update the
 * current-desk toggle), -q (quiet errors), -N (neither change directory
 * nor update the toggle). For each desk: validates it, runs the
 * "desk-cd" hook, and (unless suppressed) sets it as the current desk.
 * Finally updates the pwd file to point at the last successfully
 * resolved desk.
 *
 * Return: EXIT_SUCCESS if every desk argument resolved cleanly, otherwise
 * EXIT_FAILURE
 */
static int _desk_cd(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    int c;
    int status;
    int retcode = ETEC_OK;
    tec_arg_t args = ARGS_INIT();
    struct tec_cli_cd_options opts;

    tec_cli_cd_option_init(&opts);
    while ((c = getopt(argvec->used, argvec->argv, ":e:hnqN")) != -1) {
        switch (c) {
        case 'h':
            opts.help = true;
            break;
        case 'n':
            opts.change_tog = false;
            break;
        case 'e':
            args.env = optarg;
            break;
        case 'q':
            opts.quiet = true;
            break;
        case 'N':
            opts.change_dir = false;
            opts.change_tog = false;
            break;
        case ':':
            TEC_LOG_E(FMT_OPT_ARG_REQ, optopt);
            return tec_cli_help_usage("desk-cd");
        default:
            TEC_LOG_E(FMT_OPT_ARG_INV, optopt);
            return tec_cli_help_usage("desk-cd");
        }
    }
    argvec->i = optind;

    if (opts.help == true)
        return tec_cli_help_usage("desk-cd");
    else if ((status = tec_cli_check_env(&args, cfg))) {
        args.env = args.env ? args.env : ETEC_NOCURR;
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_DESK_CD, args.env, tec_strerror(status));
        return EXIT_FAILURE;
    } else if ((status = tec_aux_check_cd_alias(argvec)))
        return TEC_LOG_E(tec_strerror(status));

    /* Resolve alias '-' to switch to previous environment.  */
    if (argvec->argv[argvec->i] && strcmp("-", argvec->argv[argvec->i]) == 0) {
        if ((status = toggle_desk_get_prev(cfg->base.task, &args)))
            return TEC_LOG_E(EFMT_DESK_CD, ETEC_NOPREV, tec_strerror(status));
        argvec_replace(argvec, argvec->i, args.desk);
    }

    do {
        args.desk = argvec->argv[argvec->i];

        if ((status = tec_cli_check_desk(&args, cfg))) {
            args.desk = args.desk ? args.desk : ETEC_NOCURR;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_CD, args.desk, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        }

        if ((status = hook_action(&args, "desk-cd", cfg))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_DESK_CD, args.task, tec_strerror(status));
        } else if (opts.change_tog == true) {
            if ((status = toggle_desk_set_curr(cfg->base.task, &args))) {
                if (opts.quiet == false)
                    TEC_LOG_E(tec_strerror(status));
            }
        }
        RETUPD(retcode, status);
    } while (++argvec->i < argvec->used);

    if (retcode == ETEC_OK && opts.change_dir) {
        status = tec_cli_pwd_set(&args, cfg);
        RETUPD(retcode, status);
    }
    return retcode == ETEC_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

static const tec_cmd_t desk_commands[] = {
    {.name = "add",.func = &_desk_add},
    {.name = "cat",.func = &_desk_cat},
    {.name = "cd",.func = &_desk_cd},
    {.name = "ls",.func = &_desk_ls},
    {.name = "mv",.func = &_desk_mv},
    {.name = "rm",.func = &_desk_rm},
    {.name = "set",.func = &_desk_set},
};

/**
 * tec_cli_desk() - Dispatch `tec desk add/cat/cd/ls/mv/rm/set` to its subcommand handler
 * @argvec: parsed argv; argv[1] names the subcommand ("ls" if omitted)
 * @cfg: active configuration
 *
 * Return: the subcommand handler's return value, or the value of
 * TEC_LOG_E() if argv[1] doesn't match a known subcommand
 */
int tec_cli_desk(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    const char *cmd = argvec->argv[1] != NULL ? argvec->argv[1] : "ls";

    argvec_offset(argvec, 1);   /* Skip desk from argvec.  */
    for (size_t i = 0; i < ARRAY_SIZE(desk_commands); ++i) {
        if (strcmp(cmd, desk_commands[i].name) == 0) {
            return desk_commands[i].func(argvec, cfg);
        }
    }
    return TEC_LOG_E("'%s': no such desk subcommand", cmd);
}
