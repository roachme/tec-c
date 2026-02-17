#include <string.h>
#include <stdlib.h>

#include "tec.h"
#include "aux/config.h"

/* TODO:
    Structure: tec PGN -i -b -p COMMAND [OPTION]... [APRS]
    1. Can't use getopt cuz there might be different options for plugins and
       their commads.
    2. Use for/while loop to circle through options and their arguments.
    3. Separate plugin options from plugin command options.
    4. Or maybe it's better to let the plugin to handle plugin options and the rest.
*/
int tec_cli_pgn(int argc, const char **argv, tec_ctx_t *ctx)
{
    int i;
    char *name;
    tec_argvec_t argvec;
    char cmd[BUFSIZ + 1] = { 0 };
    const char *fmt = "%s/%s/%s -T %s -P %s";

    i = 0;
    argvec_init(&argvec);
    argvec_parse(&argvec, argc, argv);
    name = argvec.argv[i++];

    sprintf(cmd, fmt, teccfg.base.pgn, name, name, teccfg.base.task,
            teccfg.base.pgn);

    for (; i < argvec.used; ++i) {
        strcat(cmd, " ");
        strcat(cmd, argvec.argv[i]);
    }

    dlog(1, "pgn: %s", cmd);
    argvec_free(&argvec);
    return system(cmd);
}
