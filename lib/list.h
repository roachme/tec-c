#ifndef TEC_LIST_H
#define TEC_LIST_H

#include <stddef.h>

#include "libtec.h"

void list_free(tec_list_t * list);
tec_list_t *list_add(tec_list_t * list, char *id, int status);
size_t list_size(tec_list_t * list);

#endif
