// TODO:
#include <stdio.h>
#include <stdlib.h>

#include "osdep.h"

#ifdef __linux__

/**
 * MKDIR() - create a directory, including any missing parents
 * @path: path of the directory to create
 *
 * Shells out to `mkdir -p @path`.
 *
 * Return: false (0) if the command succeeded, true (nonzero) if
 * `mkdir` reported an error.
 */
bool MKDIR(char *path)
{
    char cmd[BUFSIZ + 1];
    sprintf(cmd, "mkdir -p %s", path);
    return system(cmd);
}

/**
 * RMDIR() - recursively remove a directory
 * @path: path of the directory to remove
 *
 * Shells out to `rm -rf @path`.
 *
 * Return: false (0) if the command succeeded, true (nonzero) if `rm`
 * reported an error.
 */
bool RMDIR(char *path)
{
    char cmd[BUFSIZ + 1];
    sprintf(cmd, "rm -rf %s", path);
    return system(cmd);
}

/**
 * MOVE() - move/rename a file or directory
 * @src: existing path to move
 * @dst: destination path
 *
 * Shells out to `mv @src @dst`.
 *
 * Return: false (0) if the command succeeded, true (nonzero) if `mv`
 * reported an error.
 */
bool MOVE(char *src, char *dst)
{
    char cmd[BUFSIZ + 1];
    sprintf(cmd, "mv %s %s", src, dst);
    return system(cmd);
}

/**
 * ISFILE() - check whether a path names a regular file
 * @fname: path to check
 *
 * Shells out to `test -f @fname`.
 *
 * Return: true if @fname is a regular file, false otherwise.
 */
bool ISFILE(char *fname)
{
    char cmd[BUFSIZ + 1];
    sprintf(cmd, "test -f %s", fname);
    return system(cmd) == EXIT_SUCCESS;
}

/**
 * ISDIR() - check whether a path names a directory
 * @fname: path to check
 *
 * Shells out to `test -d @fname`.
 *
 * Return: true if @fname is a directory, false otherwise.
 */
bool ISDIR(char *fname)
{
    char cmd[BUFSIZ + 1];
    sprintf(cmd, "test -d %s", fname);
    return system(cmd) == EXIT_SUCCESS;
}

#elif __APPLE__

/**
 * MKDIR() - create a directory, including any missing parents
 * @path: path of the directory to create
 *
 * Not yet implemented on this platform.
 *
 * Return: 0 (unconditionally).
 */
bool MKDIR(char *path)
{
    return 0;
}

/**
 * RMDIR() - recursively remove a directory
 * @path: path of the directory to remove
 *
 * Not yet implemented on this platform.
 *
 * Return: 0 (unconditionally).
 */
bool RMDIR(char *path)
{
    return 0;
}

/**
 * MOVE() - move/rename a file or directory
 * @src: existing path to move
 * @dst: destination path
 *
 * Not yet implemented on this platform.
 *
 * Return: 0 (unconditionally).
 */
bool MOVE(char *src, char *dst)
{
    return 0;
}

/**
 * ISFILE() - check whether a path names a regular file
 * @fname: path to check
 *
 * Not yet implemented on this platform.
 *
 * Return: 0 (unconditionally).
 */
bool ISFILE(char *fname)
{
    return 0;
}

/**
 * ISDIR() - check whether a path names a directory
 * @fname: path to check
 *
 * Not yet implemented on this platform.
 *
 * Return: 0 (unconditionally).
 */
bool ISDIR(char *fname)
{
    return 0;
}

#elif WIN32 || __MINGW32__

/**
 * MKDIR() - create a directory, including any missing parents
 * @path: path of the directory to create
 *
 * Not yet implemented on this platform.
 *
 * Return: 0 (unconditionally).
 */
bool MKDIR(char *path)
{
    return 0;
}

/**
 * RMDIR() - recursively remove a directory
 * @path: path of the directory to remove
 *
 * Not yet implemented on this platform.
 *
 * Return: 0 (unconditionally).
 */
bool RMDIR(char *path)
{
    return 0;
}

/**
 * MOVE() - move/rename a file or directory
 * @src: existing path to move
 * @dst: destination path
 *
 * Not yet implemented on this platform.
 *
 * Return: 0 (unconditionally).
 */
bool MOVE(char *src, char *dst)
{
    return 0;
}

/**
 * ISFILE() - check whether a path names a regular file
 * @fname: path to check
 *
 * Not yet implemented on this platform.
 *
 * Return: 0 (unconditionally).
 */
bool ISFILE(char *fname)
{
    return 0;
}

/**
 * ISDIR() - check whether a path names a directory
 * @fname: path to check
 *
 * Not yet implemented on this platform.
 *
 * Return: 0 (unconditionally).
 */
bool ISDIR(char *fname)
{
    return 0;
}

#endif
