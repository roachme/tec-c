#ifndef TEC_CLI_PWD_H
#define TEC_CLI_PWD_H

#include "../../lib/libtec.h"

int tec_cli_pwd_set(tec_arg_t * args);
int tec_cli_pwd_set_path(tec_arg_t * args, const char *path);
int tec_cli_pwd_unset(void);

#endif
