#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "log.h"
#include "argvec.h"

/**
 * argvec_init() - Initialize a dynamic argument vector
 * @vec: vector to initialize
 *
 * Allocates an initial two-slot NULL-terminated argv array. Aborts the
 * process via exit(1) if the allocation fails.
 */
void argvec_init(tec_argvec_t *vec)
{
    int size = 2;

    if ((vec->argv = malloc(size * sizeof(char *))) == NULL) {
        TEC_LOG_E("'%s': memory allocation failed", __func__);
        exit(1);
    }

    /* Prevent invalid pointer dereference. Used in case when default
     * env name, desk name, task ID used.  */
    for (int i = 0; i < size; ++i)
        vec->argv[i] = NULL;

    vec->i = 0;
    vec->used = 0;
    vec->offset = 0;
    vec->size = size;
}

/**
 * argvec_is_empty() - Check whether a vector holds no arguments
 * @vec: vector to check
 *
 * Return: true if @vec has zero used slots, false otherwise
 */
bool argvec_is_empty(const tec_argvec_t *vec)
{
    return vec->used == 0;
}

/**
 * argvec_add() - Append an argument to a dynamic vector
 * @vec: vector to append to
 * @arg: string to copy and append
 *
 * Grows the backing array (doubling its size) when full, keeping the
 * last slot reserved as a NULL terminator. @arg is duplicated with
 * strdup(); the vector owns the copy. Aborts the process via exit(1)
 * if reallocation fails.
 */
void argvec_add(tec_argvec_t *vec, const char *arg)
{
    /* The last element is never used that's why minus one.
     * It is a NULL terminator to mimic C argv.  */
    if (vec->used >= vec->size - 1) {
        vec->size *= 2;
        if ((vec->argv =
             realloc(vec->argv, vec->size * sizeof(char *))) == NULL) {
            TEC_LOG_E("'%s': memory allocation failed", __func__);
            exit(1);
        }

        /* Prevent invalid pointer dereference. Used in case when default
         * env name, desk name, task ID used.  */
        for (int i = vec->used; i < vec->size; ++i)
            vec->argv[i] = NULL;
    }
    vec->argv[vec->used++] = strdup(arg);
}

/**
 * argvec_parse() - Load a C-style argv array into a dynamic vector
 * @vec: destination vector, must already be initialized
 * @argc: number of entries in @argv
 * @argv: array of argument strings to copy in
 *
 * Calls argvec_add() for each of the @argc entries in @argv, in order.
 */
void argvec_parse(tec_argvec_t *vec, int argc, const char **argv)
{
    for (int i = 0; i < argc; i++)
        argvec_add(vec, argv[i]);
}

/**
 * argvec_replace() - Replace an existing element of a vector in place
 * @vec: vector to modify
 * @vec_idx: index of the element to replace, must be within [0, vec->used)
 * @arg: new string value; it is duplicated, the original is not retained
 *
 * Frees the previous value at @vec_idx and installs a duplicate of
 * @arg in its place. Aborts the process via exit(1) if the duplicate
 * allocation fails.
 */
void argvec_replace(tec_argvec_t *vec, int vec_idx, char *arg)
{
    int argsiz = strlen(arg);

    assert(vec_idx >= 0 && vec_idx < vec->used);
    free(vec->argv[vec_idx]);   /* free previous key value.  */
    if ((vec->argv[vec_idx] = strndup(arg, argsiz)) == NULL) {
        TEC_LOG_E("'%s': memory allocation failed", __func__);
        exit(1);
    }
}

/**
 * argvec_offset() - Advance the vector's logical start by @offset elements
 * @vec: vector to adjust
 * @offset: number of leading elements to drop from the visible range
 *
 * Moves vec->argv forward by @offset elements and shrinks vec->used by
 * the same amount, effectively hiding the first @offset arguments
 * (e.g. to skip over a consumed subcommand name). The cumulative
 * offset is tracked in vec->offset so argvec_deinit() can restore the
 * original pointer before freeing.
 */
void argvec_offset(tec_argvec_t *vec, int offset)
{
    assert((vec->used - offset) >= 0);
    vec->offset += offset;
    vec->used -= offset;
    vec->argv += offset;
}

/**
 * argvec_deinit() - Free all memory owned by a dynamic argument vector
 * @vec: vector to release
 *
 * Restores vec->argv to its original (pre-argvec_offset()) pointer,
 * frees every element string, then frees the array itself.
 */
void argvec_deinit(tec_argvec_t *vec)
{
    vec->argv = vec->argv - vec->offset;        /* Restore pointer to first element.  */
    for (int i = 0; i < vec->size; ++i)
        free(vec->argv[i]);
    free(vec->argv);
}
