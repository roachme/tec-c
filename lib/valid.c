#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "valid.h"

int is_valid_length(char *obj, int len)
{
    if (strlen(obj) <= len)
        return true;
    return false;
}

/* TODO: Maybe there's generic function to validate name.
 *       If so unify them all.  */

int valid_column_name(char *name)
{
    if (!isalnum(*name++))
        return 0;
    for (; *name; ++name)
        if (!(isalnum(*name) || *name == '_' || *name == '-'))
            return 0;
    return isalnum(*--name) != 0;
}

int valid_env_name(char *name)
{
    if (!isalnum(*name++))
        return 0;
    for (; *name; ++name)
        if (!(isalnum(*name) || *name == '_' || *name == '-'))
            return 0;
    return isalnum(*--name) != 0;
}

int valid_desk_name(char *name)
{
    if (!isalnum(*name++))
        return 0;
    for (; *name; ++name)
        if (!(isalnum(*name) || *name == '_' || *name == '-'))
            return 0;
    return isalnum(*--name) != 0;
}

int valid_task_name(char *name)
{
    if (!isalnum(*name++))
        return 0;
    for (; *name; ++name)
        if (!(isalnum(*name) || *name == '_' || *name == '-'))
            return 0;
    return isalnum(*--name) != 0;
}
