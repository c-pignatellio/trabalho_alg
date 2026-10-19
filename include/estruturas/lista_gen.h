#ifndef LISTA_GEN_H
#define LISTA_GEN_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_gen_ LISTA_GEN;

LISTA_GEN *lista_gen_criar(void);
bool lista_gen_adicionar(LISTA_GEN *lg, const char *grupo, const char *subgrupo, ITEM *item);
void lista_gen_apagar(LISTA_GEN **lg);

#endif