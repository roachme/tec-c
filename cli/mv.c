#include <string.h>
#include <stdlib.h>

#include "tec.h"
#include "aux/config.h"
#include "aux/toggle.h"

/*

Usage: tec move SRC... DST

Single move (rename):
    tec move task1 task11           -> rename task1 to task11 in current project and current board
    tec move ././task1 ././task11   -> same as above (explicit current project/board)

Multiple moves:
    tec move task1 task2 task3 otherboard/
        -> move task1, task2, task3 to otherboard (keeping same task IDs)
    tec move task1 task2 otherproj/otherboard/
        -> move tasks to different project/board

Notes:
    '.'     - current project/board/task
    '..'    - previous project/board/task
    Trailing '/' in DST indicates destination is a directory (project/board), not a task rename

'.' in arguments can be omited so use current arg by default.
*/

/*
 * Parse a path argument into tec_arg_t components.
 * Format: [project/[board/]]task or ././task
 * '.' means current, '..' means previous
 * Returns 0 on success, non-zero on error.
 */
static int parse_path(const char *path, tec_arg_t *args, const char *errfmt)
{
    char *buf, *token, *saveptr;
    char *parts[3] = { NULL, NULL, NULL };
    int nparts = 0;
    int status = 0;

    if (path == NULL || *path == '\0') {
        /* Empty path means use current for all */
        if ((status = toggle_project_get_curr(teccfg.base.task, args)))
            return elog(status, errfmt, ".", "could not get current project");
        if ((status = toggle_board_get_curr(teccfg.base.task, args)))
            return elog(status, errfmt, ".", "could not get current board");
        if ((status = toggle_task_get_curr(teccfg.base.task, args)))
            return elog(status, errfmt, ".", "could not get current task");
        return 0;
    }

    buf = strdup(path);
    if (buf == NULL)
        return elog(1, errfmt, path, "memory allocation failed");

    /* Split by '/' */
    token = strtok_r(buf, "/", &saveptr);
    while (token != NULL && nparts < 3) {
        parts[nparts++] = token;
        token = strtok_r(NULL, "/", &saveptr);
    }

    if (nparts == 1) {
        /* Just task ID: "task1" or "." or ".." */
        if ((status = toggle_project_get_curr(teccfg.base.task, args))) {
            free(buf);
            return elog(status, errfmt, ".", "could not get current project");
        }
        if ((status = toggle_board_get_curr(teccfg.base.task, args))) {
            free(buf);
            return elog(status, errfmt, ".", "could not get current board");
        }

        if (strcmp(parts[0], ".") == 0) {
            if ((status = toggle_task_get_curr(teccfg.base.task, args))) {
                free(buf);
                return elog(status, errfmt, ".", "could not get current task");
            }
        } else if (strcmp(parts[0], "..") == 0) {
            if ((status = toggle_task_get_prev(teccfg.base.task, args))) {
                free(buf);
                return elog(status, errfmt, "..",
                            "could not get previous task");
            }
        } else {
            args->taskid = strdup(parts[0]);
        }
    } else if (nparts == 2) {
        /* board/task: "board/task1" or "./." */
        if ((status = toggle_project_get_curr(teccfg.base.task, args))) {
            free(buf);
            return elog(status, errfmt, ".", "could not get current project");
        }

        if (strcmp(parts[0], ".") == 0) {
            if ((status = toggle_board_get_curr(teccfg.base.task, args))) {
                free(buf);
                return elog(status, errfmt, ".", "could not get current board");
            }
        } else if (strcmp(parts[0], "..") == 0) {
            if ((status = toggle_board_get_prev(teccfg.base.task, args))) {
                free(buf);
                return elog(status, errfmt, "..",
                            "could not get previous board");
            }
        } else {
            args->board = strdup(parts[0]);
        }

        if (strcmp(parts[1], ".") == 0) {
            if ((status = toggle_task_get_curr(teccfg.base.task, args))) {
                free(buf);
                return elog(status, errfmt, ".", "could not get current task");
            }
        } else if (strcmp(parts[1], "..") == 0) {
            if ((status = toggle_task_get_prev(teccfg.base.task, args))) {
                free(buf);
                return elog(status, errfmt, "..",
                            "could not get previous task");
            }
        } else {
            args->taskid = strdup(parts[1]);
        }
    } else if (nparts == 3) {
        /* project/board/task: "proj/board/task1" or "././." */
        if (strcmp(parts[0], ".") == 0) {
            if ((status = toggle_project_get_curr(teccfg.base.task, args))) {
                free(buf);
                return elog(status, errfmt, ".",
                            "could not get current project");
            }
        } else if (strcmp(parts[0], "..") == 0) {
            if ((status = toggle_project_get_prev(teccfg.base.task, args))) {
                free(buf);
                return elog(status, errfmt, "..",
                            "could not get previous project");
            }
        } else {
            args->project = strdup(parts[0]);
        }

        if (strcmp(parts[1], ".") == 0) {
            if ((status = toggle_board_get_curr(teccfg.base.task, args))) {
                free(buf);
                return elog(status, errfmt, ".", "could not get current board");
            }
        } else if (strcmp(parts[1], "..") == 0) {
            if ((status = toggle_board_get_prev(teccfg.base.task, args))) {
                free(buf);
                return elog(status, errfmt, "..",
                            "could not get previous board");
            }
        } else {
            args->board = strdup(parts[1]);
        }

        if (strcmp(parts[2], ".") == 0) {
            if ((status = toggle_task_get_curr(teccfg.base.task, args))) {
                free(buf);
                return elog(status, errfmt, ".", "could not get current task");
            }
        } else if (strcmp(parts[2], "..") == 0) {
            if ((status = toggle_task_get_prev(teccfg.base.task, args))) {
                free(buf);
                return elog(status, errfmt, "..",
                            "could not get previous task");
            }
        } else {
            args->taskid = strdup(parts[2]);
        }
    }

    free(buf);
    return 0;
}

/*
 * Parse destination path which may be a directory (ends with '/') or a task path.
 * If is_dir is set to true, the destination is a directory and taskid should not be set.
 */
static int parse_dest(const char *path, tec_arg_t *args, int *is_dir,
                      const char *errfmt)
{
    size_t len;

    *is_dir = false;

    if (path == NULL || *path == '\0')
        return parse_path(path, args, errfmt);

    len = strlen(path);

    /* Check if destination ends with '/' indicating it's a directory */
    if (path[len - 1] == '/') {
        char *buf = strdup(path);
        if (buf == NULL)
            return elog(1, errfmt, path, "memory allocation failed");

        buf[len - 1] = '\0';    /* Remove trailing slash */
        *is_dir = true;

        /* Now parse the path without trailing slash */
        /* Count slashes to determine if it's project, board, or project/board */
        int slashes = 0;
        for (size_t j = 0; j < len - 1; j++)
            if (buf[j] == '/')
                slashes++;

        if (slashes == 0) {
            /* Just "board/" - set as destination board */
            if (strcmp(buf, ".") == 0) {
                if (toggle_board_get_curr(teccfg.base.task, args)) {
                    free(buf);
                    return elog(1, errfmt, ".", "could not get current board");
                }
            } else if (strcmp(buf, "..") == 0) {
                if (toggle_board_get_prev(teccfg.base.task, args)) {
                    free(buf);
                    return elog(1, errfmt, "..",
                                "could not get previous board");
                }
            } else {
                args->board = strdup(buf);
            }
            /* Use current project */
            if (toggle_project_get_curr(teccfg.base.task, args)) {
                free(buf);
                return elog(1, errfmt, ".", "could not get current project");
            }
        } else if (slashes == 1) {
            /* "project/board/" */
            char *slash = strchr(buf, '/');
            *slash = '\0';
            char *proj = buf;
            char *board = slash + 1;

            if (strcmp(proj, ".") == 0) {
                if (toggle_project_get_curr(teccfg.base.task, args)) {
                    free(buf);
                    return elog(1, errfmt, ".",
                                "could not get current project");
                }
            } else if (strcmp(proj, "..") == 0) {
                if (toggle_project_get_prev(teccfg.base.task, args)) {
                    free(buf);
                    return elog(1, errfmt, "..",
                                "could not get previous project");
                }
            } else {
                args->project = strdup(proj);
            }

            if (strcmp(board, ".") == 0) {
                if (toggle_board_get_curr(teccfg.base.task, args)) {
                    free(buf);
                    return elog(1, errfmt, ".", "could not get current board");
                }
            } else if (strcmp(board, "..") == 0) {
                if (toggle_board_get_prev(teccfg.base.task, args)) {
                    free(buf);
                    return elog(1, errfmt, "..",
                                "could not get previous board");
                }
            } else {
                args->board = strdup(board);
            }
        }

        free(buf);
        return 0;
    }

    /* Not a directory, parse as regular path */
    return parse_path(path, args, errfmt);
}

int tec_cli_mv(int argc, char **argv, tec_ctx_t *ctx)
{
    char c;
    int i, showhelp, status, nargs, is_dir;
    tec_arg_t dst, src;
    char *errfmt;

    showhelp = false;
    is_dir = false;
    errfmt = "cannot parse '%s': %s";
    src.project = src.board = src.taskid = NULL;
    dst.project = dst.board = dst.taskid = NULL;

    while ((c = getopt(argc, argv, ":ft:h")) != -1) {
        switch (c) {
        case 'f':
            return elog(1, "option `-f' under development");
            break;
        case 'h':
            showhelp = true;
            break;
        case 't':
            return elog(1, "option `-t' under development");
            break;
        case ':':
            return elog(1, "option `-%c' requires an argument", optopt);
        default:
            return elog(1, "invalid option `-%c'", optopt);
        }
    }

    if (showhelp)
        return help_usage("move");

    i = optind;
    nargs = argc - i;

    if (nargs < 1) {
        return elog(1, "source task is missing");
    }

    if (nargs < 2) {
        return elog(1, "destination is missing");
    }

    /* Parse destination (last argument) */
    if ((status = parse_dest(argv[argc - 1], &dst, &is_dir, errfmt)))
        return status;

    if (nargs == 2 && !is_dir) {
        /* Single move: tec move src dst */
        if ((status = parse_path(argv[i], &src, errfmt)))
            return status;

        /* If destination has no project/board, inherit from source */
        if (dst.project == NULL)
            dst.project = src.project;
        if (dst.board == NULL)
            dst.board = src.board;

        if ((status = tec_task_move(teccfg.base.task, &src, &dst, ctx))) {
            return elog(status, "could not (re)move '%s': %s", src.taskid,
                        tec_strerror(status));
        }

        /* Update toggles after successful move */
        if (strcmp(src.project, dst.project) == 0 &&
            strcmp(src.board, dst.board) == 0) {
            /* Same board: rename - update task ID in toggles */
            if (strcmp(src.taskid, dst.taskid) != 0)
                toggle_task_update(teccfg.base.task, &src, src.taskid,
                                   dst.taskid);
        } else {
            /* Different board/project: clear from source toggles */
            toggle_task_clear(teccfg.base.task, &src, src.taskid);
        }
    } else {
        /* Multiple moves: tec move src1 src2 ... dst/ */
        int last_status = 0;

        /* Iterate over all source arguments (all except the last one) */
        for (; i < argc - 1; i++) {
            /* Reset src for each iteration */
            src.project = src.board = src.taskid = NULL;

            if ((status = parse_path(argv[i], &src, errfmt))) {
                last_status = status;
                continue;
            }

            /* For directory destination, use source task ID as destination task ID */
            tec_arg_t move_dst;
            move_dst.project = dst.project ? dst.project : src.project;
            move_dst.board = dst.board ? dst.board : src.board;
            move_dst.taskid = src.taskid;       /* Keep same task ID */

            if ((status =
                 tec_task_move(teccfg.base.task, &src, &move_dst, ctx))) {
                elog(status, "could not move '%s': %s", src.taskid,
                     tec_strerror(status));
                last_status = status;
            } else {
                /* Update toggles after successful move */
                if (strcmp(src.project, move_dst.project) == 0 &&
                    strcmp(src.board, move_dst.board) == 0) {
                    /* Same board: rename - update task ID in toggles */
                    if (strcmp(src.taskid, move_dst.taskid) != 0)
                        toggle_task_update(teccfg.base.task, &src,
                                           src.taskid, move_dst.taskid);
                } else {
                    /* Different board/project: clear from source toggles */
                    toggle_task_clear(teccfg.base.task, &src, src.taskid);
                }
            }
        }
        return last_status;
    }

    return 0;
}
