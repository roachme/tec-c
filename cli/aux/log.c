#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include "log.h"
#include "config.h"

/**
 * tec_cli_log_error() - Print an error message to stderr, prefixed with the program name
 * @fmt: printf-style format string
 * @...: arguments for @fmt
 *
 * Return: always EXIT_FAILURE, so callers can write "return TEC_LOG_E(...)"
 */
int tec_cli_log_error(const char *fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);
    fprintf(stderr, "%s: ", PROGRAM);
    vfprintf(stderr, fmt, arg);
    fprintf(stderr, "\n");
    va_end(arg);
    return EXIT_FAILURE;
}

/**
 * tec_cli_log_info() - Print an informational message to stdout
 * @fmt: printf-style format string
 * @...: arguments for @fmt
 *
 * Return: always EXIT_SUCCESS
 */
int tec_cli_log_info(const char *fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stdout, fmt, arg);
    fprintf(stdout, "\n");
    va_end(arg);
    return EXIT_SUCCESS;
}

/**
 * tec_cli_log_debug() - Print a source-tagged debug message to stderr, if debugging is enabled
 * @fname: source file name to tag the message with (typically __FILE__)
 * @line: source line number to tag the message with (typically __LINE__)
 * @fmt: printf-style format string
 * @...: arguments for @fmt
 *
 * No-op unless teccfg.opts.debug is true.
 *
 * Return: always EXIT_SUCCESS
 */
int tec_cli_log_debug(const char *fname, int line, const char *fmt, ...)
{
    va_list arg;

    if (teccfg.opts.debug == false)
        return EXIT_SUCCESS;

    va_start(arg, fmt);
    fprintf(stderr, "%s[%s:%d]: ", PROGRAM, fname, line);
    vfprintf(stderr, fmt, arg);
    fprintf(stderr, "\n");
    va_end(arg);
    return EXIT_SUCCESS;
}

/**
 * tec_cli_log_prompt() - Print a prompt message to stderr, unadorned
 * @fmt: printf-style format string
 * @...: arguments for @fmt
 *
 * Unlike tec_cli_log_error(), does not prefix the program name or
 * append a trailing newline, so it can be used to print an inline
 * prompt before reading a response.
 *
 * Return: always EXIT_SUCCESS
 */
int tec_cli_log_prompt(const char *fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stderr, fmt, arg);
    va_end(arg);
    return EXIT_SUCCESS;
}
