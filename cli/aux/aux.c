#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "aux.h"
#include "errno.h"
#include "osdep.h"
#include "config.h"

char *unitkeys[] = { "prio", "type", "date", "desc", };

unsigned int nunitkey = sizeof(unitkeys) / sizeof(unitkeys[0]);

/**
 * tec_aux_yesno() - Prompt-read a yes/no answer from stdin
 *
 * Reads a single line from stdin. Only an initial 'y' or 'Y' counts
 * as an affirmative answer; anything else (including EOF/read
 * failure, which leaves the buffer untouched) is treated as "no".
 *
 * Return: true if the first character read is 'y' or 'Y', false otherwise
 */
bool tec_aux_yesno(void)
{
    char choice[10] = { 0 };

    fgets(choice, sizeof(choice), stdin);
    if (choice[0] == 'y' || choice[0] == 'Y')
        return true;
    return false;
}

/**
 * tec_aux_do_change_user_cwd() - Check whether the shell is already inside the target task dir
 * @args: env/desk/task selection whose on-disk task directory is checked
 *
 * Builds the expected task directory path from teccfg.base.task and
 * @args, and compares it against the shell's current working
 * directory (via tec_cli_osdep_getenv_cwd()).
 *
 * Return: true if the process' current working directory already
 * matches the task's directory, false otherwise
 */
bool tec_aux_do_change_user_cwd(tec_arg_t *args)
{
    char *base = teccfg.base.task;
    char buf[FILENAME_MAX + 1] = { 0 };

    sprintf(buf, "%s/%s/%s/%s", base, args->env, args->desk, args->task);
    return strcmp(buf, tec_cli_osdep_getenv_cwd()) == 0;
}

/**
 * tec_aux_check_cd_alias() - Reject the "-" cd alias when combined with other arguments
 * @argvec: remaining CLI argument vector to scan from its current index
 *
 * Scans the unconsumed portion of @argvec (from argvec->i to
 * argvec->used) for a literal "-" argument that is not the sole
 * remaining argument.
 *
 * Return: ETEC_ALIAS if "-" is present together with other trailing
 * arguments, ETEC_OK otherwise
 */
int tec_aux_check_cd_alias(tec_argvec_t *argvec)
{
    for (int idx = argvec->i; idx < argvec->used; ++idx) {
        if (strcmp(argvec->argv[idx], "-") == 0 && argvec->used - argvec->i > 1)
            return ETEC_ALIAS;
    }
    return ETEC_OK;
}

/**
 * tec_aux_is_valid_desc() - Validate a description string
 * @val: NUL-terminated string to validate
 *
 * A valid description must start and end with an alphanumeric
 * character; every character in between must be alphanumeric,
 * whitespace, '_', or '-'.
 *
 * Return: non-zero (true) if @val is valid, 0 (false) otherwise
 */
int tec_aux_is_valid_desc(const char *val)
{
    if (!isalnum(*val++))
        return false;
    for (; *val; ++val)
        if (!(isalnum(*val) || isspace(*val) || *val == '_' || *val == '-'))
            return false;
    return isalnum(*--val) != 0;
}

/* roachme: replace all prios if user specifies any in config file */
/**
 * tec_aux_is_valid_prio() - Check whether a string names a known priority level
 * @val: NUL-terminated candidate priority string
 *
 * Compares @val (up to 10 characters) against the fixed set
 * "lowest", "low", "mid", "high", "highest".
 *
 * Return: true if @val matches one of the known priority names, false otherwise
 */
int tec_aux_is_valid_prio(const char *val)
{
    char *prios[] = { "lowest", "low", "mid", "high", "highest" };
    int size = sizeof(prios) / sizeof(prios[0]);

    for (int i = 0; i < size; ++i)
        if (strncmp(val, prios[i], 10) == 0)
            return true;
    return false;
}

/* roachme: replace all types if user specifies any in config file */
/**
 * tec_aux_is_valid_type() - Check whether a string names a known task type
 * @val: NUL-terminated candidate type string
 *
 * Compares @val (up to 10 characters) against the fixed set "task",
 * "bugfix", "feature", "hotfix".
 *
 * Return: true if @val matches one of the known type names, false otherwise
 */
int tec_aux_is_valid_type(const char *val)
{
    char *types[] = { "task", "bugfix", "feature", "hotfix" };
    int size = sizeof(types) / sizeof(types[0]);

    for (int i = 0; i < size; ++i) {
        if (strncmp(val, types[i], 10) == 0)
            return true;
    }
    return false;
}

/**
 * aux_show_key() - Print the value of a single unit key
 * @key: key name to look up
 * @units: linked list of key/value units to search
 *
 * Walks @units looking for an entry whose key matches @key and, if
 * found, prints its value to stdout followed by a newline.
 *
 * Return: EXIT_SUCCESS if @key was found and printed, EXIT_FAILURE otherwise
 */
int aux_show_key(char *key, tec_unit_t *units)
{
    for (; units; units = units->next) {
        if (!strcmp(key, units->key)) {
            printf("%s\n", units->val);
            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}
