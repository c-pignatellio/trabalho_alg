#ifndef LISTA_ENC_H
#define LISTA_ENC_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_enc_ LISTA_ENC;
typedef void (*CallbackEnc)(ITEM *item, void *contexto);

LISTA_ENC *lista_enc_criar(void);
bool lista_enc_inserir(LISTA_ENC *l, ITEM *item);
int lista_enc_tamanho(const LISTA_ENC *l);
void lista_enc_percorrer(const LISTA_ENC *l, CallbackEnc funcao, void *contexto);
void lista_enc_apagar(LISTA_ENC **l);

#endif