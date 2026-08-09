#include <stdlib.h>
#include "osdep.h"

#ifdef __linux__
#include <unistd.h>
#endif

#ifdef __linux__
/**
 * tec_cli_osdep_getenv() - Linux implementation: read an environment variable
 * @name: environment variable name to look up
 *
 * Reads from the shell environment rather than resolving the actual
 * OS current working directory, because the latter may differ if the
 * shell's $PWD is a symlink.
 *
 * Return: pointer to the variable's value (owned by the environment,
 * not to be freed), or NULL if it is not set
 */
static char *tec_cli_osdep_getenv(char *name)
{
    /* Get logical current working directory via shell variable, because
     * user current working directory might a symlink.  */
    return getenv(name);
}
#endif

#ifdef __linux__
/**
 * tec_cli_osdep_chdir() - Linux implementation: change the process' working directory
 * @path: directory to change into
 *
 * Return: 0 on success, -1 on failure (errno set by chdir())
 */
int tec_cli_osdep_chdir(char *path)
{
    return chdir(path);
}
#endif

/**
 * tec_cli_osdep_getenv_home() - Get the user's home directory
 *
 * Return: pointer to the HOME environment variable's value, or NULL
 * if unset
 */
char *tec_cli_osdep_getenv_home(void)
{
    return tec_cli_osdep_getenv("HOME");
}

/**
 * tec_cli_osdep_getenv_cwd() - Get the shell's logical current working directory
 *
 * Return: pointer to the PWD environment variable's value, or NULL if unset
 */
char *tec_cli_osdep_getenv_cwd(void)
{
    return tec_cli_osdep_getenv("PWD");
}
