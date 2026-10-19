#ifndef FILA_H
#define FILA_H

#include <stdbool.h>
#include "item.h"

typedef struct fila_ FILA;

FILA *fila_criar(void);
bool fila_enfileirar(FILA *f, ITEM *item);
ITEM *fila_desenfileirar(FILA *f);
ITEM *fila_frente(const FILA *f);
bool fila_vazia(const FILA *f);
int fila_tamanho(const FILA *f);
void fila_apagar(FILA **f);

#endif