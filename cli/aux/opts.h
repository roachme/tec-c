#ifndef TEC_CLI_OPTS_H
#define TEC_CLI_OPTS_H

#include <stdbool.h>
#include "../tec.h"

/**
 * enum tec_cli_rm_mode - when `rm` should prompt before removing
 * @RMI_ALWAYS: always prompt before removing
 * @RMI_SOMETIMES: prompt only in certain cases (e.g. non-empty)
 * @RMI_NEVER: never prompt, remove unconditionally
 */
enum tec_cli_rm_mode {
    RMI_ALWAYS,
    RMI_SOMETIMES,
    RMI_NEVER,
};

/**
 * struct tec_cli_cat_options - parsed options for the `cat` command
 * @help: output a usage message and exit
 * @quiet: do not write anything to standard output
 */
struct tec_cli_cat_options {
    bool help;                  /* Output a usage message ad exit */
    bool quiet;                 /* Do not write anything to standard output */
};

/**
 * struct tec_cli_cd_options - parsed options for the `cd` command
 * @help: output a usage message and exit
 * @quiet: do not write anything to standard output
 * @verbose: explain what is being done
 * @change_dir: change the shell's working directory
 * @change_tog: update the curr/prev toggles
 * @path: subdirectory inside the task dir to cd into, or NULL
 */
struct tec_cli_cd_options {
    bool help;                  /* Output a usage message ad exit */
    bool quiet;                 /* Do not write anything to standard output */
    bool verbose;               /* Explain what is being done */
    bool change_dir;            /* Change working directory */
    bool change_tog;            /* Update toggles */
    const char *path;           /* Subdirectory inside the task dir to cd into */
};

/**
 * struct tec_cli_ls_options - parsed options for the `ls` command
 * @help: output a usage message and exit
 * @quiet: do not write anything to standard output
 * @togg: show only toggle objects
 * @head: show headers
 */
struct tec_cli_ls_options {
    bool help;                  /* Output a usage message ad exit */
    bool quiet;                 /* Do not write anything to standard output */
    bool togg;                  /* Show only toggle objects */
    bool head;                  /* Show headers */
};

/**
 * struct tec_cli_rm_options - parsed options for the `rm` command
 * @help: output a usage message and exit
 * @quiet: do not write anything to standard output
 * @verbose: explain what is being done
 * @change_dir: change working directory, chosen automatically
 * @mode: RMI_ALWAYS/RMI_SOMETIMES/RMI_NEVER — when to prompt before removing
 */
struct tec_cli_rm_options {
    bool help;                  /* Output a usage message ad exit */
    bool quiet;                 /* Do not write anything to standard output */
    bool verbose;               /* Explain what is being done */
    bool change_dir;            /* Change working directory, chosen automatically */
    enum tec_cli_rm_mode mode;  /* Set remove mode */
};

/**
 * struct tec_cli_set_options - parsed options for the `set` command
 * @help: output a usage message and exit
 * @quiet: do not write anything to standard output
 */
struct tec_cli_set_options {
    bool help;                  /* Output a usage message ad exit */
    bool quiet;                 /* Do not write anything to standard output */
};

void tec_cli_option_init(tec_cfg_t * cfg);
void tec_cli_cat_option_init(struct tec_cli_cat_options *opts);
void tec_cli_cd_option_init(struct tec_cli_cd_options *opts);
void tec_cli_ls_option_init(struct tec_cli_ls_options *opts);
void tec_cli_rm_option_init(struct tec_cli_rm_options *opts);
void tec_cli_set_option_init(struct tec_cli_set_options *opts);

#endif
