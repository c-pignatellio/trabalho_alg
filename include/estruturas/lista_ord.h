#ifndef LISTA_ORD_H
#define LISTA_ORD_H

#include <stdbool.h>
#include <stddef.h>
#include "item.h"

typedef struct lista_ord_ LISTA_ORD;

LISTA_ORD *lista_ord_criar(void);
bool lista_ord_inserir(LISTA_ORD *l, ITEM *item, double chave_ordenacao);
size_t lista_ord_tamanho(const LISTA_ORD *l);
void lista_ord_percorrer(LISTA_ORD *l, void (*cb)(ITEM*, void*), void *ctx);
void lista_ord_destruir(LISTA_ORD *l, bool owns_items);

#endif