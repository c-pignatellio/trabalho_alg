#ifndef LISTA_ORD_H
#define LISTA_ORD_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_ord_ LISTA_ORD;
typedef void (*CallbackOrd)(ITEM *item, void *contexto);

LISTA_ORD *lista_ord_criar(void);
bool lista_ord_inserir(LISTA_ORD *l, ITEM *item, double chave_ordenacao);
int lista_ord_tamanho(const LISTA_ORD *l);
void lista_ord_percorrer(const LISTA_ORD *l, CallbackOrd funcao, void *contexto);
void lista_ord_apagar(LISTA_ORD **l);

#endif