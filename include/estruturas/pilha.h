#ifndef PILHA_H
#define PILHA_H

#include <stdbool.h>
#include "item.h"

typedef struct pilha_ PILHA;

PILHA *pilha_criar(void);
bool pilha_empilhar(PILHA *p, ITEM *item);
ITEM *pilha_desempilhar(PILHA *p);
ITEM *pilha_topo(const PILHA *p);
bool pilha_vazia(const PILHA *p);
int pilha_tamanho(const PILHA *p);
void pilha_apagar(PILHA **p);

#endif