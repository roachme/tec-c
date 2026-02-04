#include <string.h>

#include "tec.h"
#include "aux/toggle.h"
#include "aux/config.h"

static const char *errfmt = "cannot list tasks '%s': %s";

/*
Column names:
    todo
    ...
    ...
    ...
    done

Options:
    -a      show alls
    -t      show only toggles
    -c COL  show tasks on specific column

Note:
    Tasks should be sorted by columns.
*/

struct list_filter {
    int all;
    int toggle;
    char *column;
};

static struct list_filter filter = {
    .all = false,
    .toggle = false,
    .column = NULL,
};

static int check_filters(void)
{
    if (filter.toggle && filter.column)
        return elog(1, "options `-%s' and `-%s' are not compatible", "t", "c");
    else if (filter.toggle && filter.all)
        return elog(1, "options `-%s' and `-%s' are not compatible", "t", "a");
    else if (filter.column && filter.all)
        return elog(1, "options `-%s' and `-%s' are not compatible", "c", "a");
    return 0;
}

// TODO: remove parameter 'quiet', return status, and let the caller to
static char *get_unit_desc(tec_ctx_t *ctx, tec_arg_t *args, int quiet)
{
    int status;
    char *desc;

    desc = NULL;
    if ((status = tec_task_get(teccfg.base.task, args, ctx))) {
        if (quiet == false)
            elog(status, "'%s': %s one", args->taskid, tec_strerror(status));
    } else if ((desc = tec_unit_key(ctx->units, "desc")) == NULL) {
        if (quiet == false)
            elog(1, "'%s': %s", args->taskid, "description not found");
    }
    return desc;
}

static char *get_column_name(tec_ctx_t *ctx, tec_arg_t *args, int quiet)
{
    int status;
    char *colname;

    colname = NULL;
    if ((status = tec_task_column_get(teccfg.base.task, args, ctx))) {
        if (quiet == false)
            elog(status, "'%s': %s one", args->taskid, tec_strerror(status));
    } else if ((colname = tec_unit_key(ctx->column, "name")) == NULL) {
        if (quiet == false)
            elog(1, "'%s': %s", args->taskid, "column name not found");
    }
    return colname;
}

static void show_column(tec_ctx_t *ctx, tec_arg_t *args, tec_list_t *obj,
                        int quiet)
{
    if (obj != NULL) {
        char *desc;
        char *colname;
        args->taskid = obj->name;

        if ((desc = get_unit_desc(ctx, args, quiet)) == NULL)
            return;
        else if ((colname = get_column_name(ctx, args, quiet)) == NULL)
            return;

        if (filter.column && strcmp(filter.column, colname) == 0) {
            LIST_OBJ_UNITS(colname, obj->name, "", desc);
        } else if (filter.all == true) {
            LIST_OBJ_UNITS(colname, obj->name, "", desc);
        } else if (filter.column == NULL && strcmp(colname, "done") != 0) {
            LIST_OBJ_UNITS(colname, obj->name, "", desc);
        }

        ctx->units = tec_unit_free(ctx->units);
        ctx->column = tec_unit_free(ctx->column);
    }
}

static int show_toggles(tec_ctx_t *ctx, tec_arg_t *args)
{
    int status;
    tec_list_t obj;

    args->taskid = NULL;
    if ((status = toggle_task_get_curr(teccfg.base.task, args)) == 0) {
        obj.next = NULL;
        obj.status = LIBTEC_OK;
        obj.name = args->taskid;
        show_column(ctx, args, &obj, false);
    }

    args->taskid = NULL;
    if ((status = toggle_task_get_prev(teccfg.base.task, args)) == 0) {
        obj.next = NULL;
        obj.status = LIBTEC_OK;
        obj.name = args->taskid;
        show_column(ctx, args, &obj, false);
    }
    return status;
}

static void show_columns(tec_ctx_t *ctx, tec_arg_t *args,
                         tec_list_t *list, int quiet)
{
    tec_list_t *obj;

    for (obj = list; obj != NULL; obj = obj->next) {
        show_column(ctx, args, obj, quiet);
    }
}

// TODO: Find a good error message in case option fails.  */
int tec_cli_ls(int argc, const char **argv, tec_ctx_t *ctx)
{
    char c;
    tec_arg_t args;
    tec_argvec_t argvec;
    int i, quiet, show_headers, status;

    quiet = show_headers = false;
    args.env = args.desk = args.taskid = NULL;

    argvec_init(&argvec);
    argvec_parse(&argvec, argc, argv);
    while ((c = getopt(argvec.count, argvec.argv, ":ad:c:hqvtH")) != -1) {
        switch (c) {
        case 'a':
            filter.all = true;
            break;
        case 'd':
            args.desk = optarg;
            break;
        case 'c':
            filter.column = optarg;
            break;
        case 'h':
            return help_usage("ls");
        case 'q':
            quiet = true;
            break;
        case 'v':
            return elog(1, "option `-%c' under development", c);
        case 't':
            filter.toggle = true;
            break;
        case 'H':
            show_headers = true;
            break;
        case ':':
            return elog(1, "option `-%c' requires an argument", optopt);
        default:
            return elog(1, "invalid option `-%c'", optopt);
        }
    }

    if (check_filters())
        return 1;

    i = optind;
    do {
        args.env = argvec.argv[i];

        if ((status = check_arg_env(&args, errfmt, quiet)))
            continue;
        else if ((status = check_arg_desk(&args, errfmt, quiet)))
            continue;
        else if ((status = tec_task_list(teccfg.base.task, &args, ctx))) {
            if (quiet == false)
                elog(status, errfmt, args.env, tec_strerror(status));
            continue;
        }

        if (show_headers == true)
            printf("Environment: %s\n", args.env);

        // TODO: add hooks
        // TODO: optimize object traverse (traverse multiple times)
        // TODO: optimize data structure load (it uses too much malloc)

        if (filter.toggle) {
            show_toggles(ctx, &args);
        } else if (filter.column) {
            show_columns(ctx, &args, ctx->list, quiet);
        } else {
            show_columns(ctx, &args, ctx->list, quiet);
        }

        ctx->list = tec_list_free(ctx->list);

        // HOTFIX: cuz I've no clue how to sync desk feature into envs.
        args.desk = NULL;
    } while (++i < argvec.count);

    argvec_free(&argvec);
    return status;
}
