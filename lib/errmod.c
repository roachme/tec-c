#include <string.h>

#include "libtec.h"
#include "errmod.h"

static int errcode = ETEC_OK;

const char *errcodes[__ETEC_STATUS_LAST] = {
    [ETEC_OK] = "OK",

    [ETEC_SYS_DB] = "database directory not found",
    [ETEC_SYS_MALLOC] = "cannot malloc memory",

    [ETEC_ARG_TASK_EXIST] = "task ID already exists",
    [ETEC_ARG_TASK_ILLEG] = "illegal task ID",
    [ETEC_ARG_TASK_NOSUCH] = "no such task ID",

    [ETEC_ARG_DESK_EXIST] = "desk already exists",
    [ETEC_ARG_DESK_ILLEG] = "illegal desk name",
    [ETEC_ARG_DESK_NOSUCH] = "no such desk",

    [ETEC_ARG_ENV_EXIST] = "environment already exists",
    [ETEC_ARG_ENV_ILLEG] = "illegal environment name",
    [ETEC_ARG_ENV_NOSUCH] = "no such environment",

    [ETEC_DIR_RM] = "cannot remove directory",
    [ETEC_DIR_MAKE] = "cannot create directory",
    [ETEC_DIR_MOVE] = "cannot rename directory",
    [ETEC_DIR_OPEN] = "cannot open directory",

    [ETEC_UNIT_ADD] = "cannot add unit node",
    [ETEC_UNIT_RM] = "cannot remove unit node",
    [ETEC_UNIT_GET] = "cannot get unit values",
    [ETEC_UNIT_ILLEG] = "illegal unit value",
    [ETEC_UNIT_KEY] = "unit key does not exist",
    [ETEC_UNIT_LOAD] = "cannot load units",
    [ETEC_UNIT_SAVE] = "cannot save unit values",
    [ETEC_UNIT_SET] = "cannot set unit values",
};

/**
 * emod_set() - record the current error code and return it
 * @errnum: an ETEC_* error code (or ETEC_OK) to store as the current error
 *
 * Stores @errnum in the module-level error state so it can be reported
 * later. If @errnum is negative or larger than __ETEC_STATUS_LAST, the
 * stored value is forced to -1 instead.
 *
 * Return: the value actually stored: @errnum when it is a valid code,
 * or -1 when @errnum was out of range.
 */
int emod_set(int errnum)
{
    errcode = errnum;
    if (errcode < 0 || errcode > __ETEC_STATUS_LAST)
        errcode = -1;
    return errcode;
}

/**
 * emod_geterr() - get the human-readable message for an error code
 * @errnum: an ETEC_* error code (or ETEC_OK) to translate
 *
 * Return: a pointer to a static buffer holding the message for @errnum,
 * or "internal unknown error" if @errnum is out of range. The buffer is
 * overwritten on the next call.
 */
char *emod_geterr(int errnum)
{
    static char errmsg[ERRMOD_MSGSIZ + 1];

    if (errnum < 0 || errnum >= __ETEC_STATUS_LAST)
        strncpy(errmsg, "internal unknown error", ERRMOD_MSGSIZ);
    else
        strncpy(errmsg, errcodes[errnum], ERRMOD_MSGSIZ);
    errmsg[ERRMOD_MSGSIZ] = '\0';
    return errmsg;
}
