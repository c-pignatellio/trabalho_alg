#ifndef LISTA_CABECA_H
#define LISTA_CABECA_H

#include <stdbool.h>
#include <stddef.h>
#include "item.h"

typedef struct lista_cabeca_ LISTA_CABECA;

LISTA_CABECA *lista_cabeca_criar(void);
bool lista_cabeca_inserir(LISTA_CABECA *l, ITEM *item);
size_t lista_cabeca_tamanho(const LISTA_CABECA *l);
void lista_cabeca_percorrer(LISTA_CABECA *l, void (*cb)(ITEM*, void*), void *ctx);
void lista_cabeca_destruir(LISTA_CABECA *l, bool owns_items);

#endif