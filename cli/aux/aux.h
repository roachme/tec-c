#ifndef TEC_CLI_AUX_H
#define TEC_CLI_AUX_H

#include <stdbool.h>
#include "../../lib/libtec.h"
#include "argvec.h"

#define PROGRAM     "tec"

/**
 * RETUPD() - Fold a new status into an accumulated return code
 * @retcode: lvalue holding the accumulated return code
 * @status: latest status to fold in
 *
 * Leaves @retcode unchanged if @status is ETEC_OK, otherwise
 * overwrites @retcode with @status. Used to remember the first/last
 * failure across a sequence of otherwise-independent operations.
 */
/* Update return code with status code */
#define RETUPD(retcode, status)                                 \
    do {                                                        \
        (retcode) = (status) == ETEC_OK ? (retcode) : (status); \
    } while (0)

/**
 * ARRAY_SIZE() - Number of elements in a fixed-size array
 * @x: array (not a pointer) whose element count is computed
 *
 * Return: element count of @x
 */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
/**
 * xstr() - Stringize the expanded value of a macro
 * @s: macro or token to expand and stringize
 *
 * Return: string literal of @s after one level of macro expansion
 */
#define xstr(s) str(s)
/**
 * str() - Stringize a token without expanding it first
 * @s: token to stringize
 *
 * Return: string literal of @s as written, with no macro expansion
 */
#define str(s) #s

// TODO: Get rid of it (set options with default boolean values)
#define NONEBOOL        -1      /* Not yet set boolean value */

extern char *unitkeys[];
extern unsigned int nunitkey;

bool tec_aux_yesno(void);
bool tec_aux_do_change_user_cwd(tec_arg_t * args);

int tec_cli_help_list(void);
int tec_cli_help_usage(const char *cmd);
int tec_aux_check_cd_alias(tec_argvec_t * argvec);

int tec_aux_is_valid_desc(const char *val);
int tec_aux_is_valid_prio(const char *val);
int tec_aux_is_valid_type(const char *val);

int aux_show_key(char *key, tec_unit_t * units);

#endif
