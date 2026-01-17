#ifndef LIBTEC_DIR_H
#define LIBTEC_DIR_H

#include "libtec.h"

int dir_env_add(const char *taskdir, tec_arg_t * args);
int dir_env_del(const char *taskdir, tec_arg_t * args);
int dir_env_rename(const char *taskdir, tec_arg_t * src, tec_arg_t * dst);

int dir_desk_add(const char *taskdir, tec_arg_t * args);
int dir_desk_del(const char *taskdir, tec_arg_t * args);
int dir_desk_move(const char *taskdir, tec_arg_t * src, tec_arg_t * dst);

int dir_task_add(const char *taskdir, tec_arg_t * args);
int dir_task_del(const char *taskdir, tec_arg_t * args);
int dir_task_move(const char *taskdir, tec_arg_t * src, tec_arg_t * dst);

#endif
