#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "color.h"
#include "config.h"

/* Got this module from GNU grep */

static const char *sgr_start = "\33[%sm\33[K";
static const char *sgr_end = "\33[m\33[K";

/* Start a colorized text attribute on stdout using the SGR_START
   format; the attribute is specified by SGR_SEQ.  */
/**
 * print_start_colorize() - Emit the SGR start escape sequence
 * @_sgr_start: printf-style format string embedding the SGR sequence
 * @sgr_seq: SGR attribute code(s) to insert into @_sgr_start
 *
 * Writes directly to stdout via printf().
 */
static void print_start_colorize(char const *_sgr_start, char const *sgr_seq)
{
    printf(_sgr_start, sgr_seq);
}

/* Restore the normal text attribute using the SGR_END string.  */
/**
 * print_end_colorize() - Emit the SGR reset escape sequence
 * @_sgr_end: literal SGR end sequence to write to stdout
 *
 * Writes directly to stdout via fputs().
 */
static void print_end_colorize(char const *_sgr_end)
{
    fputs(_sgr_end, stdout);
}

/* SGR utility functions.  */
/* NOTE: pro'ly variable 's' is a color string.  */
/**
 * pr_sgr_start() - Start an SGR color attribute if one is given
 * @s: color/attribute code string; a no-op if empty
 */
static void pr_sgr_start(char const *s)
{
    if (*s)
        print_start_colorize(sgr_start, s);
}

/**
 * pr_sgr_end() - Reset the SGR color attribute if @s was non-empty
 * @s: the same color/attribute code string passed to pr_sgr_start();
 *     only its emptiness is checked, its contents are otherwise unused
 */
static void pr_sgr_end(char const *s)
{
    if (*s)
        print_end_colorize(sgr_end);
}

/**
 * color_print_str() - Print a string to stdout, optionally colorized
 * @fmt: printf-style format string, must contain exactly one %s for @str
 * @str: string to print via @fmt
 * @color: SGR color/attribute code to wrap @str in when coloring is applied
 * @enabled: colorize only when non-zero (true)
 *
 * Coloring is only emitted when @enabled is true and stdout is a
 * terminal (isatty()); otherwise @str is printed plain via @fmt.
 */
void color_print_str(const char *fmt, char *str, char *color, int enabled)
{
    if (isatty(STDOUT_FILENO) && enabled == true)
        pr_sgr_start(color);
    printf(fmt, str);
    if (isatty(STDOUT_FILENO) && enabled == true)
        pr_sgr_end(CRESET);
}

/*
void color_print_char(const char *fmt, char c, char *color)
{
    if (isatty(STDOUT_FILENO) && teccfg.opts.color == true)
        pr_sgr_start(color);
    printf(fmt, c);
    if (isatty(STDOUT_FILENO) && teccfg.opts.color == true)
        pr_sgr_end(CRESET);
}
*/
