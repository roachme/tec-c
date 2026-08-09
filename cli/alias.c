#include <string.h>

#include "tec.h"
#include "aux/config.h"

// TODO: Resolve nested alias, i.e. alias that includes alias
/*
static int alias_resolve()
{
    return 0;
}
*/

/**
 * get_alias() - Look up a configured alias by name
 * @cmdname: name to search for
 * @cfg: active configuration, holding the linked list of configured aliases
 *
 * Return: pointer to the matching tec_alias_t, or NULL if not found
 */
static tec_alias_t *get_alias(const char *cmdname, tec_cfg_t *cfg)
{
    tec_alias_t *head;

    for (head = cfg->alias; head != NULL; head = head->next)
        if (strcmp(cmdname, head->name) == 0)
            return head;
    return NULL;
}

/**
 * resolve_alias() - Expand an alias's stored command string into @argvec
 * @argvec: argument vector to rewrite; its argv[0] is replaced by the
 *          alias's own command name and each further whitespace-separated
 *          token from @alias->cmd is appended
 * @alias: the resolved alias, whose ->cmd string is tokenized in place
 *         (via strtok(), which mutates it)
 *
 * Return: the alias's resolved command name (same as the new argv[0])
 */
static char *resolve_alias(tec_argvec_t *argvec, tec_alias_t *alias)
{
    char *tok, *cmdname;

    tok = cmdname = strtok(alias->cmd, " ");
    argvec_replace(argvec, 0, tok);

    while ((tok = strtok(NULL, " ")) != NULL)
        argvec_add(argvec, tok);
    return cmdname;
}

/**
 * tec_cli_alias() - Resolve and run the alias named by argv[0]
 * @argvec: parsed argv; argv[0] is the alias name, and is rewritten in
 *          place with the alias's expanded command and arguments
 * @cfg: active configuration, used to look up the alias and (if the
 *       resolved command is a plugin) the plugin directory
 *
 * Looks the alias up with get_alias(), expands it into @argvec with
 * resolve_alias(), then re-resolves the expanded command as either a
 * plugin or a builtin and runs it via tec_cli_cmd_run().
 *
 * Return: the value of TEC_LOG_E() if the alias cannot be found or
 * resolved, ETEC_OK if the expanded command matches neither a plugin nor
 * a builtin, or otherwise the status returned by tec_cli_cmd_run()
 */
int tec_cli_alias(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    tec_cmd_t *cmd;
    int status = ETEC_OK;
    tec_alias_t *alias = NULL;
    char *cmdname = argvec->argv[0];

    if ((alias = get_alias(cmdname, cfg)) == NULL) {
        return TEC_LOG_E("'%s': cannot get alias", cmdname);
    } else if ((cmdname = resolve_alias(argvec, alias)) == NULL) {
        return TEC_LOG_E("'%s': cannot resolve alias", cmdname);
    }

    if ((cmd = tec_cli_is_plugin(argvec, cfg))) {
        TEC_LOG_D("alias execute as plugin: '%s'", alias->name);
        status = tec_cli_cmd_run(cmd, argvec, cfg);
    } else if ((cmd = tec_cli_is_builtin(argvec, NULL))) {
        TEC_LOG_D("alias execute as builtin: '%s'", alias->name);
        status = tec_cli_cmd_run(cmd, argvec, cfg);
    }
    return status;
}
