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
    5. Add a better parser to include all the other tec options like -C, -H, -F, etc.
*/
int tec_cli_plugin(int argc, const char **argv, tec_ctx_t *ctx)
{
    int i;
    char *pgn;
    tec_argvec_t argvec;
    char pgnexec[BUFSIZ + 1] = { 0 };

    argvec_init(&argvec);
    argvec_parse(&argvec, argc, argv);

    i = 0;
    pgn = argvec.argv[i++];

    sprintf(pgnexec, "%s/%s/%s -T %s -P %s ", teccfg.base.pgn, pgn, pgn,
            teccfg.base.task, teccfg.base.pgn);

    for (; i < argc; ++i) {
        strcat(pgnexec, argv[i]);
        strcat(pgnexec, " ");
    }

    dlog(1, "pgnexec: %s", pgnexec);
    return system(pgnexec);
}
