#ifndef LIBTEC_PATH_H
#define LIBTEC_PATH_H

#include "libtec.h"

char *path_env_dir(const char *taskdir, const tec_arg_t * args);
char *path_board_dir(const char *taskdir, const tec_arg_t * args);
char *path_task_dir(const char *taskdir, const tec_arg_t * args);
char *path_env_board(const char *taskdir, const tec_arg_t * args);

char *path_env_unit(const char *taskdir, const tec_arg_t * args);
char *path_board_unit(const char *taskdir, const tec_arg_t * args);
char *path_task_unit(const char *taskdir, const tec_arg_t * args);

char *path_env_column(const char *taskdir, const tec_arg_t * args);
char *path_board_column(const char *taskdir, const tec_arg_t * args);
char *path_task_column(const char *taskdir, const tec_arg_t * args);

#endif
