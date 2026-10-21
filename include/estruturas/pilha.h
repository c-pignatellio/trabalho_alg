#ifndef PILHA_H
#define PILHA_H

#include <stdbool.h>
#include <stddef.h>
#include "item.h"

typedef struct pilha_ PILHA;

PILHA *pilha_criar(void);
bool pilha_inserir(PILHA *p, ITEM *item); 
ITEM *pilha_remover(PILHA *p);   
ITEM *pilha_topo(const PILHA *p);
bool pilha_vazia(const PILHA *p);
size_t pilha_tamanho(const PILHA *p);
void pilha_percorrer(PILHA *p, void (*cb)(ITEM*, void*), void *ctx);
void pilha_destruir(PILHA *p, bool owns_items);

#endif