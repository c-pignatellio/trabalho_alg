#ifndef LISTA_SEQ_H
#define LISTA_SEQ_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_seq_ LISTA_SEQ;
typedef void (*CallbackSeq)(ITEM *item, void *contexto);

LISTA_SEQ *lista_seq_criar(int capacidade);
bool lista_seq_inserir(LISTA_SEQ *l, ITEM *item);
ITEM *lista_seq_obter(const LISTA_SEQ *l, int idx);
int lista_seq_tamanho(const LISTA_SEQ *l);
void lista_seq_percorrer(const LISTA_SEQ *l, CallbackSeq funcao, void *contexto);
void lista_seq_apagar(LISTA_SEQ **l);

#endif