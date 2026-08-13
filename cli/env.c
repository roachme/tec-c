#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "tec.h"
#include "aux/log.h"
#include "../lib/list.h"
#include "aux/aux.h"
#include "aux/opts.h"
#include "aux/errno.h"
#include "aux/argvec.h"
#include "aux/config.h"
#include "aux/toggle.h"

// TODO: unify it (env, desk, task)
/**
 * valid_unitkeys() - Check that an env's unit list only contains "desc"
 * @units: linked list of env units to validate, e.g. as returned by
 *         tec_env_get()
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

/**
 * generate_units() - Build the default unit set for a newly added environment
 * @ctx: ->units is set to the newly-built unit list (just "desc")
 * @env: environment name used in the auto-generated description
 *
 * Return: 0 on success, 1 if the resulting unit list is empty
 */
static int generate_units(tec_ctx_t *ctx, char *env)
{
    struct tec_unit *units = NULL;
    char desc[100] = "Generated desciption for environment ";

    strcat(desc, env);
    units = tec_unit_add(units, "desc", desc);

    if ((ctx->units = units) == NULL)
        return 1;
    return 0;
}

/**
 * env_add_default_desk() - Create the env's default desk via `desk add`
 * @args: ->env names the just-added environment, ->desk names the desk
 *        to create in it
 * @cfg: active configuration
 * @quiet: forward -q to the synthesized `desk add` invocation
 *
 * Creates the env's default desk by delegating to `desk add` itself,
 * instead of duplicating its unit-generation/validation logic here. This
 * also gives the default desk its own generated description rather than
 * reusing the env's. Builds a synthetic argv ("desk add -e ENV -N [-q]
 * DESK") and runs it through tec_cli_desk(); -N is passed because this
 * function's caller (_env_add()) owns updating toggles/pwd itself, not
 * the delegated desk-add call. Resets getopt()'s internal scan state
 * (optind = 0, glibc's documented full-reinit) before the nested call so
 * a second invocation in the same loop iteration doesn't read stale state
 * left by the previous argv.
 *
 * Return: ETEC_OK on success, ETEC_UNIT_GEN_FAIL if the delegated
 * `desk add` did not return EXIT_SUCCESS
 */
static int env_add_default_desk(tec_arg_t *args, tec_cfg_t *cfg, bool quiet)
{
    int status;
    tec_argvec_t dargv;

    argvec_init(&dargv);
    argvec_add(&dargv, "desk");
    argvec_add(&dargv, "add");
    argvec_add(&dargv, "-e");
    argvec_add(&dargv, args->env);
    argvec_add(&dargv, "-N");   /* env add owns toggles/pwd, not desk add.  */
    if (quiet)
        argvec_add(&dargv, "-q");
    argvec_add(&dargv, args->desk);

    /* getopt() keeps global scan state (including glibc's internal
     * nextchar pointer) across calls. optind=1 alone only resets the
     * index, not that internal state, so a second call in this same
     * loop would read stale data left by the previous argv. optind=0
     * is glibc's documented way to force a full reinitialization for
     * a brand new argv.  */
    optind = 0;
    status = tec_cli_desk(&dargv, cfg);
    argvec_deinit(&dargv);
    return status == EXIT_SUCCESS ? ETEC_OK : ETEC_UNIT_GEN_FAIL;
}

/**
 * _env_add() - Implement `tec env add`, creating one or more environments
 * @argvec: parsed argv (subcommand name already skipped); remaining
 *          positional args are the environment names to add, processed in order
 * @cfg: active configuration
 *
 * Recognizes -d DESK (name for the auto-created default desk, "desk" if
 * omitted), -h (help), -n (don't update the current-env/desk toggles),
 * -q (quiet errors), -D (not yet implemented, always errors), -N
 * (neither change directory nor update the toggles). For each env name:
 * validates its length and format (tec_cli_len_valid()/tec_env_valid()),
 * rejects it if it already exists, builds its default units
 * (generate_units()), creates it via tec_env_add(), then creates its
 * default desk via env_add_default_desk(). After the loop, unless
 * suppressed, sets the last processed env/desk as current, then
 * refreshes the pwd file.
 *
 * Return: EXIT_SUCCESS if every environment was added cleanly, otherwise
 * EXIT_FAILURE (or 1 if updating the current-env/desk toggle fails)
 */
static int _env_add(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    char c;
    int status = ETEC_OK;
    int retcode = ETEC_OK;
    tec_ctx_t ctx = CTX_INIT();
    tec_arg_t args = ARGS_INIT();
    struct tec_cli_cd_options opts;

    tec_cli_cd_option_init(&opts);
    while ((c = getopt(argvec->used, argvec->argv, ":d:hnqD:N")) != -1) {
        switch (c) {
        case 'd':
            args.desk = optarg;
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
            return TEC_LOG_E("'%c': this option is not implemented yet", c);
        case 'N':
            opts.change_dir = false;
            opts.change_tog = false;
            break;
        case ':':
            TEC_LOG_E(FMT_OPT_ARG_REQ, optopt);
            return tec_cli_help_usage("env-add");
        default:
            TEC_LOG_E(FMT_OPT_ARG_INV, optopt);
            return tec_cli_help_usage("env-add");
        }
    }
    argvec->i = optind;

    if (opts.help == true)
        return tec_cli_help_usage("env-add");
    else if (optind == argvec->used)
        return TEC_LOG_E(tec_strerror(ETEC_ARG_ENV_REQ));

    /* Set default desk name to create.  */
    if (args.desk == NULL)
        args.desk = "desk";

    for (int i = argvec->i; i < argvec->used; ++i) {
        args.env = argvec->argv[i];

        if (tec_cli_len_valid(args.env, ENVSIZ) == false) {
            status = ETEC_ARG_ENV_TOO_LONG;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_ADD, args.env, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        } else if ((status = tec_env_valid(cfg->base.task, &args))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_ADD, args.env, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        } else if (!(status = tec_env_exist(cfg->base.task, &args))) {
            status = ETEC_ARG_ENV_EXIST;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_ADD, args.env, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        }

        if (generate_units(&ctx, args.env)) {
            status = ETEC_UNIT_GEN_FAIL;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_ADD, args.env, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        }

        if ((status = tec_env_add(cfg->base.task, &args, &ctx))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_ADD, argvec->argv[i], tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        }
        ctx.units = tec_unit_free(ctx.units);

        /* `desk add` reports its own errors (desk validation, hooks,
         * etc.) unless quiet, so don't duplicate a message here.  */
        if ((status = env_add_default_desk(&args, cfg, opts.quiet))) {
            retcode = EXIT_FAILURE;
            continue;
        }
    }

    if ((opts.change_tog && status == ETEC_OK)
        && toggle_env_set_curr(cfg->base.task, &args)) {
        if (opts.quiet == false)
            TEC_LOG_E(tec_strerror(status));
        return 1;
    } else if ((opts.change_tog && status == ETEC_OK)
               && toggle_desk_set_curr(cfg->base.task, &args)) {
        if (opts.quiet == false)
            TEC_LOG_E(tec_strerror(status));
        return 1;
    }

    if (retcode == ETEC_OK && opts.change_dir) {
        status = tec_cli_pwd_set(&args, cfg);
        RETUPD(retcode, status);
    }
    return retcode == ETEC_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**
 * _env_rm() - Implement `tec env rm`, removing one or more environments
 * @argvec: parsed argv (subcommand name already skipped); remaining
 *          positional args are the environment names to remove, processed
 *          in order
 * @cfg: active configuration
 *
 * Recognizes -d (explicit desk, unused for env resolution here), -f
 * (never prompt, RMI_NEVER), -h (help), -i (always prompt per env,
 * RMI_ALWAYS), -q (quiet errors), -v (verbose, log each removal), -I
 * (prompt once up front when removing more than 3 envs, RMI_SOMETIMES;
 * the default mode). For each env: runs the "env-rm" hook then removes
 * it via tec_env_rm(). Afterwards refreshes the pwd file if needed.
 *
 * Return: EXIT_SUCCESS if every environment was removed (or skipped by
 * the user at a prompt) cleanly, otherwise EXIT_FAILURE
 */
static int _env_rm(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    int c;
    int ntasks;
    int status = ETEC_OK;
    int retcode = ETEC_OK;
    tec_ctx_t ctx = CTX_INIT();
    tec_arg_t args = ARGS_INIT();
    struct tec_cli_rm_options opts;

    tec_cli_rm_option_init(&opts);
    while ((c = getopt(argvec->used, argvec->argv, ":d:fhiqvI")) != -1) {
        switch (c) {
        case 'd':
            args.desk = optarg;
            break;
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
            return tec_cli_help_usage("env-rm");
        default:
            TEC_LOG_E(FMT_OPT_ARG_INV, optopt);
            return tec_cli_help_usage("env-rm");
        }
    }
    argvec->i = optind;
    ntasks = argvec->used - argvec->i;

    if (opts.help)
        return tec_cli_help_usage("env-rm");
    else if (ntasks > 3 && opts.mode == RMI_SOMETIMES) {
        TEC_LOG_P("remove %d environments? [y/N] ", ntasks);
        if (!tec_aux_yesno()) {
            return EXIT_SUCCESS;
        }
    }

    do {
        args.env = argvec->argv[argvec->i];

        if ((status = tec_cli_check_env(&args, cfg))) {
            args.env = args.env ? args.env : ETEC_NOCURR;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_RM, args.env, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        } else if (opts.mode == RMI_ALWAYS) {
            TEC_LOG_P("remove environment '%s'? [y/N] ", args.env);
            if (!tec_aux_yesno())
                continue;
        }

        if ((status = hook_action(&args, "env-rm", cfg))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_RM, args.env, tec_strerror(status));
        } else if ((status = tec_env_rm(cfg->base.task, &args, &ctx))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_RM, args.env, tec_strerror(status));
        }
        // TODO: update current directory if current env got deleted.

        if (status == ETEC_OK && opts.verbose == true)
            TEC_LOG_I("removed environment '%s'", args.env);
        RETUPD(retcode, status);
    } while (++argvec->i < argvec->used);

    if (retcode == ETEC_OK && opts.change_dir) {
        args.env = NULL;        /* Force to get current environment.  */
        status = tec_cli_pwd_set(&args, cfg);
        if (toggle_env_get_curr(cfg->base.task, &args))
            args.env = "";
        RETUPD(retcode, status);
    }
    return retcode == ETEC_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**
 * cmp_env_id() - qsort() comparator ordering tec_list_t entries by env name
 * @a: pointer to a tec_list_t element
 * @b: pointer to a tec_list_t element
 *
 * Return: result of strcmp() on the two entries' ->name fields
 */
static int cmp_env_id(const void *a, const void *b)
{
    const tec_list_t *ea = a;
    const tec_list_t *eb = b;

    return strcmp(ea->name, eb->name);
}

/**
 * _env_ls() - Implement `tec env ls`, listing every environment
 * @argvec: parsed argv (subcommand name already skipped)
 * @cfg: active configuration
 *
 * Recognizes -h (help), -q (quiet errors), -t (only show the
 * current/previous toggled environments, opts.togg), -v (unimplemented,
 * always errors). Fetches the full env list via tec_env_list(), sorts it
 * by name (cmp_env_id()), then prints each environment's ID and
 * description, filtered to the toggled ones when -t is given.
 *
 * Return: EXIT_SUCCESS if listing and every environment's lookup
 * succeeded, otherwise EXIT_FAILURE
 */
static int _env_ls(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    int c;
    int status;
    char *desc = NULL;
    int retcode = ETEC_OK;
    tec_ctx_t ctx = CTX_INIT();
    tec_arg_t args = ARGS_INIT();
    struct tec_cli_ls_options opts;

    tec_cli_ls_option_init(&opts);
    while ((c = getopt(argvec->used, argvec->argv, ":hqtv")) != -1) {
        switch (c) {
        case 'q':
            opts.quiet = true;
            break;
        case 'h':
            opts.help = true;
            break;
        case 't':
            opts.togg = true;
            break;
        case 'v':
            return TEC_LOG_E("'-%c' is under development", c);
        case ':':
            TEC_LOG_E(FMT_OPT_ARG_REQ, optopt);
            return tec_cli_help_usage("env-ls");
        default:
            TEC_LOG_E(FMT_OPT_ARG_INV, optopt);
            return tec_cli_help_usage("env-ls");
        }
    }
    argvec->i = optind;

    if (opts.help == true) {
        return tec_cli_help_usage("env-ls");
    } else if ((retcode = tec_env_list(cfg->base.task, &args, &ctx))) {
        if (opts.quiet == false)
            return TEC_LOG_E(EFMT_ENV_LS, "ENV", tec_strerror(retcode));
    }

    if (list_size(ctx.list) > 0)
        qsort(ctx.list->items, ctx.list->count, sizeof(*ctx.list->items),
              cmp_env_id);

    for (size_t idx = 0; idx < list_size(ctx.list); idx++) {
        tec_list_t *obj = &ctx.list->items[idx];

        args.env = obj->name;

        if ((status = tec_env_get(cfg->base.task, &args, &ctx))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_LS, args.env, tec_strerror(status));
            continue;
        } else if ((desc = tec_unit_get(ctx.units, "desc")) == NULL) {
            retcode = EXIT_FAILURE;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_LS, args.env, "description not found");
            continue;
        }

        if (opts.togg && toggle_env_is_curr(cfg->base.task, &args)) {
            LIST_OBJ_UNITS(obj->name, "", desc, ENVSIZ, cfg->opts.color);
        } else if (opts.togg && toggle_env_is_prev(cfg->base.task, &args)) {
            LIST_OBJ_UNITS(obj->name, "", desc, ENVSIZ, cfg->opts.color);
        } else if (!opts.togg) {
            LIST_OBJ_UNITS(obj->name, "", desc, ENVSIZ, cfg->opts.color);
        }

        RETUPD(retcode, status);
        ctx.units = tec_unit_free(ctx.units);
    }

    ctx.list = tec_list_free(ctx.list);
    return retcode == ETEC_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**
 * _env_rename() - Implement `tec env rename SRC DST`
 * @argvec: parsed argv (subcommand name already skipped); expects exactly
 *          two positional args, the source and destination env names
 * @cfg: active configuration
 *
 * Recognizes -h (help), -q (quiet errors). Validates the source
 * environment and that the destination doesn't already exist, renames it
 * via tec_env_rename(), runs the "env-rename" hook, updates any
 * current/previous env toggle pointing at the old name via
 * toggle_env_update(), then refreshes the pwd file.
 *
 * Return: EXIT_FAILURE if argument count/validation fails; otherwise the
 * ETEC_* status from tec_env_rename(), hook_action(), or
 * toggle_env_update() on failure, or the value of tec_cli_pwd_set() on success
 */
static int _env_rename(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    int c;
    int status;
    tec_arg_t src = ARGS_INIT();
    tec_arg_t dst = ARGS_INIT();
    struct tec_cli_rm_options opts;

    tec_cli_rm_option_init(&opts);
    while ((c = getopt(argvec->used, argvec->argv, ":hq")) != -1) {
        switch (c) {
        case 'h':
            opts.help = true;
            break;
        case 'q':
            opts.quiet = true;
            break;
        default:
            TEC_LOG_E(FMT_OPT_ARG_INV, optopt);
            return tec_cli_help_usage("env-rename");
        }
    }

    if (opts.help)
        return tec_cli_help_usage("env-rename");
    else if (argvec->used - optind != 2)
        return TEC_LOG_E("source or destination env name missing");

    src.env = argvec->argv[optind];
    dst.env = argvec->argv[optind + 1];

    if ((status = tec_cli_check_env(&src, cfg))) {
        src.env = src.env ? src.env : ETEC_NOCURR;
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_ENV_REN, src.env, tec_strerror(status));
        return EXIT_FAILURE;
    } else if ((status = tec_cli_check_env(&src, cfg))) {
        src.env = src.env ? src.env : ETEC_NOCURR;
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_ENV_REN, src.env, tec_strerror(status));
        return EXIT_FAILURE;
    } else if (!tec_env_exist(cfg->base.task, &dst)) {
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_ENV_REN, dst.env, "destination env already exists");
        return EXIT_FAILURE;
    }

    if ((status = tec_env_rename(cfg->base.task, &src, &dst, NULL))) {
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_ENV_REN, "ENV", tec_strerror(status));
        return status;
    } else if ((status = hook_action(&dst, "env-rename", cfg))) {
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_ENV_REN, dst.env, tec_strerror(status));
        return status;
    } else
        if ((status =
             toggle_env_update(cfg->base.task, &dst, src.env, dst.env))) {
        if (opts.quiet == false)
            TEC_LOG_E(EFMT_ENV_REN, dst.env, tec_strerror(status));
        return status;
    }
    // FIXME: unify return code logic with the rest of the commands
    return tec_cli_pwd_set(&dst, cfg);
}

/**
 * _env_set() - Implement `tec env set`, setting the "desc" unit on one or more environments
 * @argvec: parsed argv (subcommand name already skipped); remaining
 *          positional args are the env names to update, processed in order
 * @cfg: active configuration
 *
 * Recognizes -d (explicit desk, unused for env resolution here), -h
 * (help), -q (quiet errors), -D DESC (staged "desc" unit, validated with
 * tec_aux_is_valid_desc()). The staged unit is applied to every
 * environment via tec_env_set(), followed by the "env-set" hook.
 *
 * Return: EXIT_SUCCESS if every environment updated cleanly, otherwise EXIT_FAILURE
 */
static int _env_set(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    int c;
    int status;
    int retcode = ETEC_OK;
    tec_ctx_t ctx = CTX_INIT();
    tec_arg_t args = ARGS_INIT();
    struct tec_cli_set_options opts;

    tec_cli_set_option_init(&opts);
    while ((c = getopt(argvec->used, argvec->argv, ":d:hqD:")) != -1) {
        switch (c) {
        case 'd':
            args.desk = optarg;
            break;
        case 'h':
            opts.help = true;
            break;
        case 'q':
            opts.quiet = true;
            break;
        case 'D':
            if (tec_aux_is_valid_desc(optarg) == false) {
                TEC_LOG_E("invalid description '%s'", optarg);
                return tec_cli_help_usage("env-set");
            }
            ctx.units = tec_unit_add(ctx.units, "desc", optarg);
            break;
        case ':':
            TEC_LOG_E(FMT_OPT_ARG_REQ, optopt);
            return tec_cli_help_usage("env-set");
        default:
            TEC_LOG_E(FMT_OPT_ARG_INV, optopt);
            return tec_cli_help_usage("env-set");
        }
    }
    argvec->i = optind;

    if (opts.help)
        return tec_cli_help_usage("env-set");

    do {
        args.env = argvec->argv[argvec->i];

        if ((status = tec_cli_check_env(&args, cfg))) {
            args.env = args.env ? args.env : ETEC_NOCURR;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_SET, args.env, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        }

        if ((status = tec_env_set(cfg->base.task, &args, &ctx))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_SET, args.env, tec_strerror(status));
        } else if ((status = hook_action(&args, "env-set", cfg))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_SET, args.task, tec_strerror(status));
        }
        RETUPD(retcode, status);
    } while (++argvec->i < argvec->used);

    ctx.units = tec_unit_free(ctx.units);
    return retcode == ETEC_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**
 * _env_cat() - Implement `tec env cat`, printing the units of one or more environments
 * @argvec: parsed argv (subcommand name already skipped); remaining
 *          positional args are the env names to cat, processed in order
 * @cfg: active configuration
 *
 * Recognizes -d (explicit desk, unused for env resolution here), -h
 * (help), -k KEY (repeatable; restrict output to specific keys), -q
 * (quiet errors). For each env, validates it, fetches its units via
 * tec_env_get(), sanity-checks their key order with valid_unitkeys(),
 * merges in any plugin-contributed units via hook_cat(), then prints
 * either every unit or just the requested -k keys.
 *
 * Return: EXIT_SUCCESS if every env and requested key resolved cleanly,
 * otherwise EXIT_FAILURE
 */
static int _env_cat(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    int c;
    int status;
    tec_argvec_t keys;
    int retcode = ETEC_OK;
    tec_unit_t *units = NULL;
    tec_ctx_t ctx = CTX_INIT();
    tec_arg_t args = ARGS_INIT();
    struct tec_cli_cd_options opts;
    struct tec_unit *unitpgn = NULL;

    argvec_init(&keys);
    tec_cli_cd_option_init(&opts);
    while ((c = getopt(argvec->used, argvec->argv, ":d:hk:q")) != -1) {
        switch (c) {
        case 'd':
            args.desk = optarg;
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
            tec_cli_help_usage("env-cat");
            retcode = EXIT_FAILURE;
            goto err;
        default:
            TEC_LOG_E(FMT_OPT_ARG_INV, optopt);
            tec_cli_help_usage("env-cat");
            retcode = EXIT_FAILURE;
            goto err;
        }
    }
    argvec->i = optind;

    if (opts.help) {
        retcode = ETEC_OK;
        tec_cli_help_usage("env-cat");
        goto err;
    }

    do {
        args.env = argvec->argv[argvec->i];

        if ((status = tec_cli_check_env(&args, cfg))) {
            args.env = args.env ? args.env : ETEC_NOCURR;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_CAT, args.env, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        } else if ((status = tec_env_get(cfg->base.task, &args, &ctx))) {
            retcode = EXIT_FAILURE;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_CAT, args.env, tec_strerror(status));
            continue;
        } else if ((status = valid_unitkeys(ctx.units))) {
            retcode = EXIT_FAILURE;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_CAT, args.env, tec_strerror(status));
            continue;
        } else if ((status = hook_cat(&unitpgn, &args, "env-cat", cfg))) {
            retcode = EXIT_FAILURE;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_CAT, args.env, tec_strerror(status));
            continue;
        }

        units = tec_unit_add(units, "id", args.env);
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
                    TEC_LOG_E(EFMT_ENV_CAT_UNIT, args.env, keys.argv[i]);
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
 * _env_cd() - Implement `tec env cd`, switching the "current" environment
 * @argvec: parsed argv (subcommand name already skipped); remaining
 *          positional args are the env names to cd into, processed in
 *          order; "-" means the previously-current environment
 * @cfg: active configuration
 *
 * Recognizes -d (explicit desk, unused for env resolution here), -h
 * (help), -n (don't update the current-env toggle), -q (quiet errors),
 * -N (neither change directory nor update the toggle). For each env:
 * validates it, runs the "env-cd" hook, and (unless suppressed) sets it
 * as the current environment. Finally updates the pwd file to point at
 * the last successfully resolved environment.
 *
 * Return: EXIT_SUCCESS if every env argument resolved cleanly, otherwise
 * EXIT_FAILURE
 */
static int _env_cd(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    int c;
    int status;
    int retcode = ETEC_OK;
    tec_arg_t args = ARGS_INIT();
    struct tec_cli_cd_options opts;

    tec_cli_cd_option_init(&opts);
    while ((c = getopt(argvec->used, argvec->argv, ":d:hnqN")) != -1) {
        switch (c) {
        case 'd':
            args.desk = optarg;
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
        case 'N':
            opts.change_dir = false;
            opts.change_tog = false;
            break;
        case ':':
            TEC_LOG_E(FMT_OPT_ARG_REQ, optopt);
            return tec_cli_help_usage("env-cd");
        default:
            TEC_LOG_E(FMT_OPT_ARG_INV, optopt);
            return tec_cli_help_usage("env-cd");
        }
    }
    argvec->i = optind;

    if (opts.help == true)
        return tec_cli_help_usage("env-cd");
    else if ((status = tec_aux_check_cd_alias(argvec)))
        return TEC_LOG_E(tec_strerror(status));

    /* Resolve alias '-' to switch to previous environment.  */
    if (argvec->argv[argvec->i] && strcmp("-", argvec->argv[argvec->i]) == 0) {
        if ((status = toggle_env_get_prev(cfg->base.task, &args)))
            return TEC_LOG_E(EFMT_ENV_CD, ETEC_NOPREV, tec_strerror(status));
        argvec_replace(argvec, argvec->i, args.env);
    }

    do {
        args.env = argvec->argv[argvec->i];

        if ((status = tec_cli_check_env(&args, cfg))) {
            args.env = args.env ? args.env : ETEC_NOCURR;
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_CD, args.env, tec_strerror(status));
            retcode = EXIT_FAILURE;
            continue;
        }

        if ((status = hook_action(&args, "env-cd", cfg))) {
            if (opts.quiet == false)
                TEC_LOG_E(EFMT_ENV_CD, args.task, tec_strerror(status));
        } else if (opts.change_tog == true) {
            if ((status = toggle_env_set_curr(cfg->base.task, &args))) {
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

static const tec_cmd_t env_commands[] = {
    {.name = "add",.func = &_env_add},
    {.name = "cat",.func = &_env_cat},
    {.name = "cd",.func = &_env_cd},
    {.name = "ls",.func = &_env_ls},
    {.name = "rename",.func = &_env_rename},
    {.name = "rm",.func = &_env_rm},
    {.name = "set",.func = &_env_set},
};

/**
 * tec_cli_env() - Dispatch `tec env add/cat/cd/ls/rename/rm/set` to its subcommand handler
 * @argvec: parsed argv; argv[1] names the subcommand ("ls" if omitted)
 * @cfg: active configuration
 *
 * Return: the subcommand handler's return value, or the value of
 * TEC_LOG_E() if argv[1] doesn't match a known subcommand
 */
int tec_cli_env(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    const char *cmd = argvec->argv[1] != NULL ? argvec->argv[1] : "ls";

    argvec_offset(argvec, 1);   /* Skip env from argvec.  */
    for (size_t i = 0; i < ARRAY_SIZE(env_commands); ++i) {
        if (strcmp(cmd, env_commands[i].name) == 0) {
            return env_commands[i].func(argvec, cfg);
        }
    }
    return TEC_LOG_E("'%s': no such env command", cmd);
}
