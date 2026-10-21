#ifndef LISTA_ENC_H
#define LISTA_ENC_H

#include <stdbool.h>
#include <stddef.h>
#include "item.h"

typedef struct lista_enc_ LISTA_ENC;

LISTA_ENC *lista_enc_criar(void);
bool lista_enc_inserir(LISTA_ENC *l, ITEM *item);
size_t lista_enc_tamanho(const LISTA_ENC *l);
void lista_enc_percorrer(LISTA_ENC *l, void (*cb)(ITEM*, void*), void *ctx);
void lista_enc_destruir(LISTA_ENC *l, bool owns_items);

#endif