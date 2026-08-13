#include <string.h>

#include "tec.h"
#include "aux/errno.h"
#include "aux/toggle.h"
#include "aux/config.h"
#include "../lib/list.h"

struct list_filter {
    int all;
    int toggle;
};

static struct list_filter filter = {
    .all = false,
    .toggle = false,
};

/**
 * check_filters() - Validate that the -t and -a filter options aren't combined
 *
 * Return: 0 if at most one of filter.toggle/filter.all is set, otherwise
 * the value of TEC_LOG_E()
 */
static int check_filters(void)
{
    if (filter.toggle && filter.all)
        return TEC_LOG_E("options `-%s' and `-%s' are not compatible", "t",
                         "a");
    return 0;
}

// TODO: remove parameter 'quiet', return status, and let the caller to
/**
 * get_unit_desc() - Fetch a task's "desc" unit value
 * @ctx: filled in by tec_task_get() with the task's units on success
 * @args: identifies the task to fetch (args->task must be set)
 * @quiet: suppress TEC_LOG_E() output when set
 * @cfg: active configuration
 *
 * Return: the task's description string, or NULL if the task couldn't be
 * fetched or has no "desc" unit
 */
static char *get_unit_desc(tec_ctx_t *ctx, tec_arg_t *args, int quiet,
                           tec_cfg_t *cfg)
{
    int status;
    char *desc = NULL;

    if ((status = tec_task_get(cfg->base.task, args, ctx))) {
        if (quiet == false)
            TEC_LOG_E("'%s': %s one", args->task, tec_strerror(status));
    } else if ((desc = tec_unit_get(ctx->units, "desc")) == NULL) {
        if (quiet == false)
            TEC_LOG_E("'%s': %s", args->task, "description not found");
    }
    return desc;
}

/**
 * show_row() - Print one task's listing row ("id  desc")
 * @ctx: scratch context passed through to get_unit_desc(); its ->units
 *       are freed after the row is printed
 * @args: scratch args; ->task is set to @obj's name for the lookup
 * @obj: the list entry to print, or NULL to do nothing
 * @quiet: suppress TEC_LOG_E() output when set
 * @cfg: active configuration
 */
static void show_row(tec_ctx_t *ctx, tec_arg_t *args, tec_list_t *obj,
                     int quiet, tec_cfg_t *cfg)
{
    if (obj != NULL) {
        char *desc = NULL;
        args->task = obj->name;

        if ((desc = get_unit_desc(ctx, args, quiet, cfg)) == NULL)
            return;

        LIST_OBJ_UNITS(obj->name, "", desc, IDSIZ, cfg->opts.color);
        ctx->units = tec_unit_free(ctx->units);
    }
}

/**
 * show_toggles() - Print the rows for the current and previous toggled tasks
 * @ctx: scratch context forwarded to show_row()
 * @args: scratch args, reused across the current/previous lookups
 *
 * Used for `ls -t`. Looks up the current-task and previous-task toggles
 * with toggle_task_get_curr()/toggle_task_get_prev() and prints a row for
 * each one found, after validating it still resolves to a real task.
 * @cfg: active configuration
 *
 * Return: the status of the last toggle_task_get_prev() call
 */
static int show_toggles(tec_ctx_t *ctx, tec_arg_t *args, tec_cfg_t *cfg)
{
    int status;
    tec_list_t obj;
    int opt_quiet = 0;          /* TODO: sync it with option passed to CLI.  */

    args->task = NULL;
    if ((status = toggle_task_get_curr(cfg->base.task, args)) == 0) {
        if ((status = tec_cli_check_task(args, cfg))) {
            if (opt_quiet == false)
                TEC_LOG_E(EFMT_TASK_LS, args->task, tec_strerror(status));
        } else {
            obj.status = ETEC_OK;
            obj.name = args->task;
            show_row(ctx, args, &obj, false, cfg);
        }
    }

    args->task = NULL;
    if ((status = toggle_task_get_prev(cfg->base.task, args)) == 0) {
        if ((status = tec_cli_check_task(args, cfg))) {
            if (opt_quiet == false)
                TEC_LOG_E(EFMT_TASK_LS, args->task, tec_strerror(status));
        } else {
            obj.status = ETEC_OK;
            obj.name = args->task;
            show_row(ctx, args, &obj, false, cfg);
        }
    }
    return status;
}

/**
 * cmp_task_id() - qsort() comparator ordering tec_list_t entries by task ID
 * @a: pointer to a tec_list_t element
 * @b: pointer to a tec_list_t element
 *
 * Return: result of strcmp() on the two entries' ->name fields
 */
static int cmp_task_id(const void *a, const void *b)
{
    const tec_list_t *ea = a;
    const tec_list_t *eb = b;

    return strcmp(ea->name, eb->name);
}

/**
 * show_rows() - Sort a task list by ID and print a row for each entry
 * @ctx: scratch context forwarded to show_row()
 * @args: scratch args forwarded to show_row()
 * @list: the task list to display, as filled in by tec_task_list()
 * @quiet: suppress TEC_LOG_E() output when set
 * @cfg: active configuration
 */
static void show_rows(tec_ctx_t *ctx, tec_arg_t *args,
                      tec_listarr_t *list, int quiet, tec_cfg_t *cfg)
{
    size_t count = list_size(list);

    if (count > 0)
        qsort(list->items, count, sizeof(*list->items), cmp_task_id);

    for (size_t i = 0; i < count; i++) {
        show_row(ctx, args, &list->items[i], quiet, cfg);
    }
}

// TODO: Find a good error message in case option fails.  */
/**
 * tec_cli_ls() - List the tasks in one or more environments
 * @argvec: parsed argv; remaining positional args (after options) are the
 *          environment names to list, processed in order
 * @cfg: active configuration
 *
 * Recognizes -a (list all, filter.all), -d DESK (explicit desk), -h
 * (help), -q (quiet errors), -v (unimplemented, always errors), -t (list
 * only the current/previous toggled tasks, filter.toggle), -H (print an
 * "Environment: ..." header before each environment's rows). -t and -a
 * are mutually exclusive (checked by check_filters()). For each
 * environment, resolves env/desk, fetches the task list via
 * tec_task_list(), and prints it with show_toggles() or show_rows().
 *
 * Return: the status of the last processed environment (ETEC_OK on
 * success; note this does not accumulate failures across multiple
 * environment arguments the way other commands' retcode does)
 */
int tec_cli_ls(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    char c;
    tec_ctx_t ctx = CTX_INIT();
    tec_arg_t args;
    int i, quiet, show_headers, status;

    quiet = show_headers = false;
    args.env = args.desk = args.task = NULL;

    while ((c = getopt(argvec->used, argvec->argv, ":ad:hqvtH")) != -1) {
        switch (c) {
        case 'a':
            filter.all = true;
            break;
        case 'd':
            args.desk = optarg;
            break;
        case 'h':
            return tec_cli_help_usage("ls");
        case 'q':
            quiet = true;
            break;
        case 'v':
            return TEC_LOG_E("option `-%c' under development", c);
        case 't':
            filter.toggle = true;
            break;
        case 'H':
            show_headers = true;
            break;
        case ':':
            TEC_LOG_E(FMT_OPT_ARG_REQ, optopt);
            return tec_cli_help_usage("ls");
        default:
            TEC_LOG_E(FMT_OPT_ARG_INV, optopt);
            return tec_cli_help_usage("ls");
        }
    }

    if (check_filters())
        return 1;

    i = optind;
    do {
        args.env = argvec->argv[i];

        if ((status = tec_cli_check_env(&args, cfg))) {
            args.env = args.env ? args.env : ETEC_NOCURR;
            if (quiet == false)
                TEC_LOG_E(EFMT_TASK_LS, args.task, tec_strerror(status));
            continue;
        } else if ((status = tec_cli_check_desk(&args, cfg))) {
            args.desk = args.desk ? args.desk : ETEC_NOCURR;
            if (quiet == false)
                TEC_LOG_E(EFMT_TASK_LS, args.task, tec_strerror(status));
            continue;
        } else if ((status = tec_task_list(cfg->base.task, &args, &ctx))) {
            if (quiet == false)
                TEC_LOG_E(EFMT_TASK_LS, args.env, tec_strerror(status));
            continue;
        }

        if (show_headers == true)
            printf("Environment: %s\n", args.env);

        // TODO: add hooks
        // TODO: optimize object traverse (traverse multiple times)
        // TODO: optimize data structure load (it uses too much malloc)

        if (filter.toggle) {
            show_toggles(&ctx, &args, cfg);
        } else {
            show_rows(&ctx, &args, ctx.list, quiet, cfg);
        }

        ctx.list = tec_list_free(ctx.list);
    } while (++i < argvec->used);

    return status;
}
