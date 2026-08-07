#include <string.h>
#include "tec.h"
#include "aux/config.h"

static int _show_aliases(tec_alias_t *aliases)
{
    tec_alias_t *alias;

    for (alias = aliases; alias; alias = alias->next)
        printf("%s\t\t: %s\n", alias->name, alias->cmd);
    return 0;
}

static int _act_hooks(struct tec_hook *hooks)
{
    struct tec_hook *hook;

    for (hook = hooks; hook; hook = hook->next) {
        if (strcmp("cat", hook->cmd) && strcmp("ls", hook->cmd))
            printf("%s.%s\n", hook->pgname, hook->pgncmd);
    }
    return 0;
}

static int _cat_hooks(struct tec_hook *hooks)
{
    struct tec_hook *hook;

    for (hook = hooks; hook; hook = hook->next)
        if (strcmp("cat", hook->cmd) == 0)
            printf("%s.%s\n", hook->pgname, hook->pgncmd);
    return 0;
}

static int _ls_hooks(struct tec_hook *hooks)
{
    struct tec_hook *hook;

    for (hook = hooks; hook; hook = hook->next)
        if (strcmp("ls", hook->cmd) == 0)
            printf("%s.%s\n", hook->pgname, hook->pgncmd);
    return 0;

}

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

static void _cfg_section(const char *title, int first, int enabled)
{
    if (!first)
        printf("\n");
    color_print_str("%s:\n", (char *)title, BCYN, enabled);
}

static void _cfg_kv(const char *key, const char *val, int enabled)
{
    color_print_str("  %-" xstr(CFG_KEYW) "s", (char *)key, BBLU, enabled);
    color_print_str(": %s\n", (char *)val, WHT, enabled);
}

// TODO: show config values from config file. Not option set via CLI
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
