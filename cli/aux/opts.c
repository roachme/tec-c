#include <stddef.h>

#include "opts.h"

/**
 * tec_cli_cat_option_init() - Set default values for `cat` command options
 * @opts: options struct to initialize
 */
void tec_cli_cat_option_init(struct tec_cli_cat_options *opts)
{
    opts->help = false;
    opts->quiet = false;
}

/**
 * tec_cli_cd_option_init() - Set default values for `cd` command options
 * @opts: options struct to initialize
 *
 * Defaults both change_dir and change_tog to true (i.e. by default
 * `cd` both changes the shell directory and updates the curr/prev
 * toggles) and path to NULL (no subdirectory).
 */
void tec_cli_cd_option_init(struct tec_cli_cd_options *opts)
{
    opts->help = false;
    opts->quiet = false;
    opts->verbose = false;
    opts->change_dir = true;
    opts->change_tog = true;
    opts->path = NULL;
}

/**
 * tec_cli_rm_option_init() - Set default values for `rm` command options
 * @opts: options struct to initialize
 *
 * Defaults mode to RMI_ALWAYS (always prompt before removing).
 */
void tec_cli_rm_option_init(struct tec_cli_rm_options *opts)
{
    opts->help = false;
    opts->quiet = false;
    opts->verbose = false;
    opts->change_dir = false;
    opts->mode = RMI_ALWAYS;
}

/**
 * tec_cli_ls_option_init() - Set default values for `ls` command options
 * @opts: options struct to initialize
 */
void tec_cli_ls_option_init(struct tec_cli_ls_options *opts)
{
    opts->help = false;
    opts->quiet = false;
    opts->togg = false;
}

/**
 * tec_cli_set_option_init() - Set default values for `set` command options
 * @opts: options struct to initialize
 */
void tec_cli_set_option_init(struct tec_cli_set_options *opts)
{
    opts->help = false;
    opts->quiet = false;
}
