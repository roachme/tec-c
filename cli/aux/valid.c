#include <string.h>
#include <stdbool.h>
#include "valid.h"
#include "errno.h"
#include "toggle.h"
#include "config.h"

/**
 * tec_cli_len_valid() - Check whether a string is within a maximum length
 * @obj: NUL-terminated string to measure
 * @len: maximum allowed length (inclusive)
 *
 * Return: true if strlen(@obj) <= @len, false otherwise
 */
int tec_cli_len_valid(const char *obj, size_t len)
{
    if (strlen(obj) <= len)
        return true;
    return false;
}

/**
 * tec_cli_check_env() - Resolve and fully validate the target environment
 * @args: env/desk/task selection; args->env is filled in from the
 *        current toggle if not already set
 *
 * Runs, in order: toggle_env_get_curr() (falls back to the current
 * toggle when args->env is NULL), tec_env_valid() (name format),
 * tec_cli_len_valid() against ENVSIZ, and tec_env_exist() (on-disk
 * presence). Stops at the first failing step.
 *
 * Return: ETEC_OK if the environment resolved and validated cleanly,
 * otherwise the ETEC_* status of whichever check failed first
 */
int tec_cli_check_env(tec_arg_t *args)
{
    int status = ETEC_OK;

    if ((status = toggle_env_get_curr(teccfg.base.task, args)))
        return status;
    else if ((status = tec_env_valid(teccfg.base.task, args)))
        return status;
    else if (tec_cli_len_valid(args->env, ENVSIZ) == false)
        return ETEC_ARG_ENV_TOO_LONG;
    else if ((status = tec_env_exist(teccfg.base.task, args)))
        return status;
    return status;
}

/**
 * tec_cli_check_desk() - Resolve and fully validate the target desk
 * @args: env/desk/task selection; args->desk is filled in from the
 *        current toggle if not already set
 *
 * Runs, in order: toggle_desk_get_curr() (falls back to the current
 * toggle when args->desk is NULL), tec_desk_valid() (name format),
 * tec_cli_len_valid() against DESKSIZ, and tec_desk_exist() (on-disk
 * presence). Stops at the first failing step.
 *
 * Return: ETEC_OK if the desk resolved and validated cleanly,
 * otherwise the ETEC_* status of whichever check failed first
 */
int tec_cli_check_desk(tec_arg_t *args)
{
    int status = ETEC_OK;

    if ((status = toggle_desk_get_curr(teccfg.base.task, args)))
        return status;
    else if ((status = tec_desk_valid(teccfg.base.task, args)))
        return status;
    else if (tec_cli_len_valid(args->desk, DESKSIZ) == false)
        return ETEC_ARG_DESK_TOO_LONG;
    else if ((status = tec_desk_exist(teccfg.base.task, args)))
        return status;
    return status;
}

/**
 * tec_cli_check_task() - Resolve and fully validate the target task
 * @args: env/desk/task selection; args->task is filled in from the
 *        current toggle if not already set
 *
 * Runs, in order: toggle_task_get_curr() (falls back to the current
 * toggle when args->task is NULL), tec_task_valid() (ID format),
 * tec_cli_len_valid() against IDSIZ, and tec_task_exist() (on-disk
 * presence). Stops at the first failing step.
 *
 * Return: ETEC_OK if the task resolved and validated cleanly,
 * otherwise the ETEC_* status of whichever check failed first
 */
int tec_cli_check_task(tec_arg_t *args)
{
    int status = ETEC_OK;

    if ((status = toggle_task_get_curr(teccfg.base.task, args)))
        return status;
    else if ((status = tec_task_valid(teccfg.base.task, args)))
        return status;
    else if (tec_cli_len_valid(args->task, IDSIZ) == false)
        return ETEC_ARG_TASK_TOO_LONG;
    else if ((status = tec_task_exist(teccfg.base.task, args)))
        return status;
    return status;
}
