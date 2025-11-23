#ifndef TEC_TOGGLE_H
#define TEC_TOGGLE_H

#include "../tec.h"

int toggle_project_swap(char *base, tec_arg_t * args);
int toggle_project_get_curr(char *base, tec_arg_t * args);
int toggle_project_set_curr(char *base, tec_arg_t * args);
int toggle_project_get_prev(char *base, tec_arg_t * args);

int toggle_board_swap(char *base, tec_arg_t * args);
int toggle_board_get_curr(char *base, tec_arg_t * args);
int toggle_board_get_prev(char *base, tec_arg_t * args);
int toggle_board_set_curr(char *base, tec_arg_t * args);

int toggle_task_swap(char *base, tec_arg_t * args);
int toggle_task_get_curr(char *base, tec_arg_t * args);
int toggle_task_get_prev(char *base, tec_arg_t * args);
int toggle_task_set_curr(char *base, tec_arg_t * args);

/* Update toggles after rename/move operations */
int toggle_task_update(char *base, tec_arg_t * args,
                       const char *old_id, const char *new_id);
int toggle_task_clear(char *base, tec_arg_t * args, const char *taskid);

#endif
