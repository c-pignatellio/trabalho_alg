#ifndef LISTA_CRUZ_H
#define LISTA_CRUZ_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_cruz_ LISTA_CRUZ;

LISTA_CRUZ *lista_cruz_criar(void);
bool lista_cruz_inserir(LISTA_CRUZ *lc, ITEM *item, const char *dim1_val, const char *dim2_val);
void lista_cruz_apagar(LISTA_CRUZ **lc);

#endif