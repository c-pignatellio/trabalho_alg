#ifndef FILA_H
#define FILA_H

#include <stdbool.h>
#include <stddef.h>
#include "item.h"

typedef struct fila_ FILA;

FILA *fila_criar(void);
bool fila_inserir(FILA *f, ITEM *item); 
ITEM *fila_remover(FILA *f);                  
ITEM *fila_frente(const FILA *f);
bool fila_vazia(const FILA *f);
size_t fila_tamanho(const FILA *f);
void fila_percorrer(FILA *f, void (*cb)(ITEM*, void*), void *ctx);
void fila_destruir(FILA *f, bool owns_items);

#endif