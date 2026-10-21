#ifndef LISTA_CRUZ_H
#define LISTA_CRUZ_H

#include <stdbool.h>
#include <stddef.h>
#include "item.h"

typedef struct lista_cruz_ LISTA_CRUZ;

LISTA_CRUZ *lista_cruz_criar(void);
bool lista_cruz_inserir(LISTA_CRUZ *lc, ITEM *item, const char *dim1_val, const char *dim2_val);
size_t lista_cruz_tamanho(const LISTA_CRUZ *lc);
void lista_cruz_percorrer(LISTA_CRUZ *lc, void (*cb)(ITEM*, void*), void *ctx);
void lista_cruz_destruir(LISTA_CRUZ *lc, bool owns_items);

#endif