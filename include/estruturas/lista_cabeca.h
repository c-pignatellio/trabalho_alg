#ifndef LISTA_CABECA_H
#define LISTA_CABECA_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_cabeca_ LISTA_CABECA;
typedef void (*CallbackCabeca)(ITEM *item, void *contexto);

LISTA_CABECA *lista_cabeca_criar(void);
bool lista_cabeca_inserir(LISTA_CABECA *l, ITEM *item);
int lista_cabeca_tamanho(const LISTA_CABECA *l);
void lista_cabeca_percorrer(const LISTA_CABECA *l, CallbackCabeca funcao, void *contexto);
void lista_cabeca_apagar(LISTA_CABECA **l);

#endif