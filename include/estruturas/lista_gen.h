#ifndef LISTA_GEN_H
#define LISTA_GEN_H

#include <stdbool.h>
#include <stddef.h>
#include "item.h"

typedef struct lista_gen_ LISTA_GEN;

LISTA_GEN *lista_gen_criar(void);
bool lista_gen_inserir(LISTA_GEN *lg, const char *grupo, const char *subgrupo, ITEM *item);
size_t lista_gen_tamanho(const LISTA_GEN *lg);
void lista_gen_percorrer(LISTA_GEN *lg, void (*cb)(ITEM*, void*), void *ctx);
void lista_gen_destruir(LISTA_GEN *lg, bool owns_items);

#endif