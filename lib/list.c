#include <string.h>
#include <stdlib.h>

#include "list.h"

#define LIST_INIT_CAP 4

void list_free(tec_list_t *list)
{
    size_t i;

    if (list == NULL)
        return;
    for (i = 0; i < list->count; i++)
        free(list->items[i].name);
    free(list->items);
    free(list);
}

/* Appends to the array; callers walk it back-to-front to get the
   same most-recently-added-first order the old prepend-based list gave.  */
tec_list_t *list_add(tec_list_t *list, char *id, int status)
{
    char *name;
    tec_listobj_t *items;

    if ((name = strdup(id)) == NULL)
        return NULL;

    if (list == NULL) {
        if ((list = malloc(sizeof(*list))) == NULL) {
            free(name);
            return NULL;
        }
        list->items = NULL;
        list->count = 0;
        list->cap = 0;
    }

    if (list->count == list->cap) {
        size_t newcap = list->cap ? list->cap * 2 : LIST_INIT_CAP;

        if ((items = realloc(list->items, newcap * sizeof(*items))) == NULL) {
            free(name);
            return NULL;
        }
        list->items = items;
        list->cap = newcap;
    }

    list->items[list->count].name = name;
    list->items[list->count].status = status;
    list->count++;
    return list;
}

size_t list_size(tec_list_t *list)
{
    return list ? list->count : 0;
}
