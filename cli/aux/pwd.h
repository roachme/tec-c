#ifndef TEC_CLI_PWD_H
#define TEC_CLI_PWD_H

#include "config.h"
#include "../../lib/libtec.h"

int tec_cli_pwd_set(tec_arg_t * args, tec_cfg_t * cfg);
int tec_cli_pwd_set_path(tec_arg_t * args, const char *path, tec_cfg_t * cfg);
int tec_cli_pwd_unset(void);

#endif
