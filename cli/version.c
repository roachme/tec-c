#include "tec.h"

int tec_cli_version(int argc, char **argv, tec_ctx_t *ctx)
{
    printf("%s version %s\n", PROGRAM, VERSION);
    return 0;
}
