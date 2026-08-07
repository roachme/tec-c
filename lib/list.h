#ifndef TEC_LIST_H
#define TEC_LIST_H

#include <stddef.h>

#include "libtec.h"

void list_free(tec_listarr_t * list);
tec_listarr_t *list_add(tec_listarr_t * list, char *id, int status);
size_t list_size(tec_listarr_t * list);

#endif
