#include <string.h>
#include <stdlib.h>

#include "list.h"

#define LIST_INIT_CAP 4

/**
 * list_free() - free a listing array and all of its item names
 * @list: listing to free, may be NULL
 *
 * Frees each item's name string, the items array, and the tec_listarr_t
 * itself. Does nothing if @list is NULL.
 */
void list_free(tec_listarr_t *list)
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
/**
 * list_add() - append an item to a listing array, allocating it if needed
 * @list: listing to append to, or NULL to allocate a new one
 * @id: name of the item to add; copied internally
 * @status: status code to store alongside the item's name
 *
 * Allocates @list (and its backing items array) on first use, growing
 * the items array by doubling capacity whenever it is full.
 *
 * Return: the (possibly newly allocated or reallocated) listing on
 * success, or NULL if allocating the name, the listing itself, or the
 * items array failed.
 */
tec_listarr_t *list_add(tec_listarr_t *list, char *id, int status)
{
    char *name;
    tec_list_t *items;

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

/**
 * list_size() - get the number of items in a listing array
 * @list: listing to inspect, may be NULL
 *
 * Return: the number of items in @list, or 0 if @list is NULL.
 */
size_t list_size(tec_listarr_t *list)
{
    return list ? list->count : 0;
}
