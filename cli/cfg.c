#include <string.h>
#include "tec.h"
#include "aux/config.h"

/**
 * _show_aliases() - Print every configured alias as "name\t\t: cmd"
 * @aliases: head of the linked list of configured aliases
 *
 * Return: 0, always
 */
static int _show_aliases(tec_alias_t *aliases)
{
    tec_alias_t *alias;

    for (alias = aliases; alias; alias = alias->next)
        printf("%s\t\t: %s\n", alias->name, alias->cmd);
    return 0;
}

/**
 * _act_hooks() - Print every configured hook that isn't a "cat" or "ls" hook
 * @hooks: head of the linked list of configured hooks
 *
 * "Act" hooks are the ones run as a side effect of a command (e.g. add/rm/
 * set/cd), as opposed to hooks that contribute output to `cat`/`ls`.
 *
 * Return: 0, always
 */
static int _act_hooks(struct tec_hook *hooks)
{
    struct tec_hook *hook;

    for (hook = hooks; hook; hook = hook->next) {
        if (strcmp("cat", hook->cmd) && strcmp("ls", hook->cmd))
            printf("%s.%s\n", hook->pgname, hook->pgncmd);
    }
    return 0;
}

/**
 * _cat_hooks() - Print every hook registered for the "cat" command
 * @hooks: head of the linked list of configured hooks
 *
 * Return: 0, always
 */
static int _cat_hooks(struct tec_hook *hooks)
{
    struct tec_hook *hook;

    for (hook = hooks; hook; hook = hook->next)
        if (strcmp("cat", hook->cmd) == 0)
            printf("%s.%s\n", hook->pgname, hook->pgncmd);
    return 0;
}

/**
 * _ls_hooks() - Print every hook registered for the "ls" command
 * @hooks: head of the linked list of configured hooks
 *
 * Return: 0, always
 */
static int _ls_hooks(struct tec_hook *hooks)
{
    struct tec_hook *hook;

    for (hook = hooks; hook; hook = hook->next)
        if (strcmp("ls", hook->cmd) == 0)
            printf("%s.%s\n", hook->pgname, hook->pgncmd);
    return 0;

}

/**
 * _cfg_get() - Implement `tec cfg get KEY...`
 * @argvec: parsed argv with the "get" subcommand at index 0 and one or
 *          more config key names following it
 * @cfg: active configuration to read from
 *
 * Recognized keys: taskbase, pgnbase, opts.color, opts.debug, opts.hook,
 * hook.cat, hook.ls, hook.act, alias. Each recognized key's value is
 * printed to stdout; unrecognized keys log an error via TEC_LOG_E() but
 * do not stop processing of the remaining keys.
 *
 * Return: the value of TEC_LOG_E() if no keys were given, otherwise 0
 */
static int _cfg_get(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    if (argvec->used == 0)
        return TEC_LOG_E("wrong number of arguments, should be at least 1");

    /* Skip first argument because it's subcommand name, i.e. get.   */
    for (int i = 1; i < argvec->used; ++i) {
        if (strcmp("taskbase", argvec->argv[i]) == 0)
            printf("%s\n", cfg->base.task);
        else if (strcmp("pgnbase", argvec->argv[i]) == 0)
            printf("%s\n", cfg->base.pgn);
        else if (strcmp("opts.color", argvec->argv[i]) == 0)
            printf("%s\n", cfg->opts.color ? "true" : "false");
        else if (strcmp("opts.debug", argvec->argv[i]) == 0)
            printf("%s\n", cfg->opts.debug ? "true" : "false");
        else if (strcmp("opts.hook", argvec->argv[i]) == 0)
            printf("%s\n", cfg->opts.hook ? "true" : "false");
        else if (strcmp("hook.cat", argvec->argv[i]) == 0)
            _cat_hooks(cfg->hooks);
        else if (strcmp("hook.ls", argvec->argv[i]) == 0)
            _ls_hooks(cfg->hooks);
        else if (strcmp("hook.act", argvec->argv[i]) == 0)
            _act_hooks(cfg->hooks);
        else if (strcmp("alias", argvec->argv[i]) == 0)
            _show_aliases(cfg->alias);
        else
            TEC_LOG_E("'%s': no such config value", argvec->argv[i]);
    }

    return 0;
}

#define CFG_KEYW        10      /* fits the widest field name + padding.  */

/**
 * _cfg_section() - Print a section heading for `tec cfg ls` output
 * @title: section title to print, e.g. "Paths"
 * @first: skip the leading blank line when this is the first section printed
 * @enabled: whether to colorize the output (forwarded to color_print_str())
 */
static void _cfg_section(const char *title, int first, int enabled)
{
    if (!first)
        printf("\n");
    color_print_str("%s:\n", (char *)title, BCYN, enabled);
}

/**
 * _cfg_kv() - Print one aligned "key: value" row for `tec cfg ls` output
 * @key: field name, left-padded to CFG_KEYW columns
 * @val: field value
 * @enabled: whether to colorize the output (forwarded to color_print_str())
 */
static void _cfg_kv(const char *key, const char *val, int enabled)
{
    color_print_str("  %-" xstr(CFG_KEYW) "s", (char *)key, BBLU, enabled);
    color_print_str(": %s\n", (char *)val, WHT, enabled);
}

// TODO: show config values from config file. Not option set via CLI
/**
 * _cfg_ls() - Implement `tec cfg ls`, printing the whole active config
 * @argvec: unused
 * @cfg: active configuration to display
 *
 * Prints "Paths", "Options", "Hooks", and "Aliases" sections, each as
 * aligned key/value rows, reflecting the config as currently loaded in
 * memory (i.e. after CLI option overrides), not directly re-read from the
 * config file.
 *
 * Return: 0, always
 */
static int _cfg_ls(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    (void)argvec;
    tec_alias_t *alias;
    struct tec_hook *hook;
    int enabled = cfg->opts.color;
    char pgnref[sizeof hook->pgname + sizeof hook->pgncmd] = { 0 };

    _cfg_section("Paths", 1, enabled);
    _cfg_kv("taskbase", cfg->base.task, enabled);
    _cfg_kv("pgnbase", cfg->base.pgn, enabled);

    _cfg_section("Options", 0, enabled);
    _cfg_kv("debug", cfg->opts.debug ? "true" : "false", enabled);
    _cfg_kv("color", cfg->opts.color ? "true" : "false", enabled);
    _cfg_kv("hook", cfg->opts.hook ? "true" : "false", enabled);

    _cfg_section("Hooks", 0, enabled);
    for (hook = cfg->hooks; hook; hook = hook->next) {
        snprintf(pgnref, sizeof pgnref, "%s.%s", hook->pgname, hook->pgncmd);
        _cfg_kv(hook->cmd, pgnref, enabled);
    }

    _cfg_section("Aliases", 0, enabled);
    for (alias = cfg->alias; alias; alias = alias->next)
        _cfg_kv(alias->name, alias->cmd, enabled);

    return 0;
}

/**
 * _cfg_set() - Implement `tec cfg set` (currently a no-op stub)
 * @argvec: unused
 * @cfg: unused
 *
 * Return: 0, always
 */
static int _cfg_set(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    (void)argvec;
    (void)cfg;
    return 0;
}

static const tec_cmd_t cfg_commands[] = {
    {.name = "get",.func = &_cfg_get},
    {.name = "ls",.func = &_cfg_ls},
    {.name = "set",.func = &_cfg_set},
};

/**
 * tec_cli_cfg() - Dispatch `tec cfg get/ls/set` to its subcommand handler
 * @argvec: parsed argv; argv[1] names the subcommand ("ls" if omitted)
 * @cfg: active configuration
 *
 * Return: the subcommand handler's return value, or the value of
 * TEC_LOG_E() if argv[1] doesn't match a known subcommand
 */
int tec_cli_cfg(tec_argvec_t *argvec, tec_cfg_t *cfg)
{
    const char *cmd = argvec->argv[1] != NULL ? argvec->argv[1] : "ls";

    argvec_offset(argvec, 1);   /* Skip cfg from argvec.  */
    for (size_t i = 0; i < ARRAY_SIZE(cfg_commands); ++i) {
        if (strcmp(cmd, cfg_commands[i].name) == 0) {
            return cfg_commands[i].func(argvec, cfg);
        }
    }
    return TEC_LOG_E("'%s': no such cfg command", cmd);
}
