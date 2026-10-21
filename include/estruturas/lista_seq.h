#ifndef LISTA_SEQ_H
#define LISTA_SEQ_H

#include <stdbool.h>
#include <stddef.h>
#include "item.h"

typedef struct lista_seq_ LISTA_SEQ;

LISTA_SEQ *lista_seq_criar(size_t capacidade);
bool lista_seq_inserir(LISTA_SEQ *l, ITEM *item);
ITEM *lista_seq_obter(const LISTA_SEQ *l, size_t idx);
size_t lista_seq_tamanho(const LISTA_SEQ *l);
void lista_seq_percorrer(LISTA_SEQ *l, void (*cb)(ITEM*, void*), void *ctx);
void lista_seq_destruir(LISTA_SEQ *l, bool owns_items);

#endif