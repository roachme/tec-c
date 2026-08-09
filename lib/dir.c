#include <limits.h>
#include <string.h>

#include "dir.h"
#include "osdep.h"
#include "path.h"

/**
 * _task_mkdb() - create the .tec metadata directory for a task
 * @taskdir: root directory of the task database
 * @args: identifiers of the task whose metadata directory is created
 *
 * Return: 0 on success, nonzero if the directory could not be created.
 */
static int _task_mkdb(const char *taskdir, tec_arg_t *args)
{
    char *path = path_task_db(taskdir, args);
    return MKDIR(path);
}

/**
 * _desk_mkdb() - create the .tec metadata directory for a desk
 * @taskdir: root directory of the task database
 * @args: identifiers of the desk whose metadata directory is created
 *
 * Return: 0 on success, nonzero if the directory could not be created.
 */
static int _desk_mkdb(const char *taskdir, tec_arg_t *args)
{
    char *path = path_desk_db(taskdir, args);
    return MKDIR(path);
}

/**
 * _env_mkdb() - create the .tec metadata directory for an environment
 * @taskdir: root directory of the task database
 * @args: identifiers of the environment whose metadata directory is created
 *
 * Return: 0 on success, nonzero if the directory could not be created.
 */
static int _env_mkdb(const char *taskdir, tec_arg_t *args)
{
    char *path = path_env_db(taskdir, args);
    return MKDIR(path);
}

/**
 * dir_task_add() - create a task's directory and its metadata directory
 * @taskdir: root directory of the task database
 * @args: identifiers of the task to create
 *
 * Return: 0 on success, nonzero if either the task directory or its
 * .tec metadata directory could not be created.
 */
int dir_task_add(const char *taskdir, tec_arg_t *args)
{
    char *path = path_task_dir(taskdir, args);
    return !(MKDIR(path) == 0 && _task_mkdb(taskdir, args) == 0);
}

/**
 * dir_task_rm() - recursively remove a task's directory
 * @taskdir: root directory of the task database
 * @args: identifiers of the task to remove
 *
 * Return: 0 on success, nonzero if the directory could not be removed.
 */
int dir_task_rm(const char *taskdir, tec_arg_t *args)
{
    char *path = path_task_dir(taskdir, args);
    return RMDIR(path);
}

/**
 * dir_task_move() - move a task's directory to a different env/desk
 * @taskdir: root directory of the task database
 * @src: identifiers of the task's current location
 * @dst: identifiers of the task's new location
 *
 * Return: 0 on success, nonzero if the directory could not be moved.
 */
int dir_task_move(const char *taskdir, tec_arg_t *src, tec_arg_t *dst)
{
    char srcpath[PATH_MAX + 1];
    char dstpath[PATH_MAX + 1];

    strcpy(srcpath, path_task_dir(taskdir, src));
    strcpy(dstpath, path_task_dir(taskdir, dst));
    return MOVE(srcpath, dstpath);
}

/**
 * dir_desk_add() - create a desk's directory and its metadata directory
 * @taskdir: root directory of the task database
 * @args: identifiers of the desk to create
 *
 * Return: 0 on success, nonzero if either the desk directory or its
 * .tec metadata directory could not be created.
 */
int dir_desk_add(const char *taskdir, tec_arg_t *args)
{
    char *path = path_desk_dir(taskdir, args);
    return !(MKDIR(path) == 0 && _desk_mkdb(taskdir, args) == 0);
}

/**
 * dir_desk_rm() - recursively remove a desk's directory
 * @taskdir: root directory of the task database
 * @args: identifiers of the desk to remove
 *
 * Return: 0 on success, nonzero if the directory could not be removed.
 */
int dir_desk_rm(const char *taskdir, tec_arg_t *args)
{
    char *path = path_desk_dir(taskdir, args);
    return RMDIR(path);
}

/**
 * dir_desk_move() - move a desk's directory to a different environment
 * @taskdir: root directory of the task database
 * @src: identifiers of the desk's current location
 * @dst: identifiers of the desk's new location
 *
 * Return: 0 on success, nonzero if the directory could not be moved.
 */
int dir_desk_move(const char *taskdir, tec_arg_t *src, tec_arg_t *dst)
{
    char srcpath[PATH_MAX + 1];
    char dstpath[PATH_MAX + 1];

    strcpy(srcpath, path_desk_dir(taskdir, src));
    strcpy(dstpath, path_desk_dir(taskdir, dst));
    return MOVE(srcpath, dstpath);
}

/**
 * dir_env_add() - create an environment's directory and its metadata directory
 * @taskdir: root directory of the task database
 * @args: identifiers of the environment to create
 *
 * Return: 0 on success, nonzero if either the environment directory or
 * its .tec metadata directory could not be created.
 */
int dir_env_add(const char *taskdir, tec_arg_t *args)
{
    char *path = path_env_dir(taskdir, args);
    return !(MKDIR(path) == 0 && _env_mkdb(taskdir, args) == 0);
}

/**
 * dir_env_rm() - recursively remove an environment's directory
 * @taskdir: root directory of the task database
 * @args: identifiers of the environment to remove
 *
 * Return: 0 on success, nonzero if the directory could not be removed.
 */
int dir_env_rm(const char *taskdir, tec_arg_t *args)
{
    char *path = path_env_dir(taskdir, args);
    return RMDIR(path);
}

/**
 * dir_env_rename() - rename an environment's directory
 * @taskdir: root directory of the task database
 * @src: identifiers holding the environment's current name
 * @dst: identifiers holding the environment's new name
 *
 * Return: 0 on success, nonzero if the directory could not be renamed.
 */
int dir_env_rename(const char *taskdir, tec_arg_t *src, tec_arg_t *dst)
{
    char srcpath[PATH_MAX + 1];
    char dstpath[PATH_MAX + 1];

    strcpy(srcpath, path_env_dir(taskdir, src));
    strcpy(dstpath, path_env_dir(taskdir, dst));
    return MOVE(srcpath, dstpath);
}
