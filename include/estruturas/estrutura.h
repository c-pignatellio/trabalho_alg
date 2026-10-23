#ifndef ESTRUTURA_H
#define ESTRUTURA_H

#include <stdbool.h>
#include <stddef.h>
#include "../item.h"
#include "../query.h"

typedef struct Estrutura Estrutura;

typedef struct {
    const char *ordem_campo;
    bool        ordem_desc;
    const char *const *group; int n_group;
    const char *const *cross; int n_cross;
    const Query *query;
} EstruturaCfg;

Estrutura *estrutura_create(const char *nome, const EstruturaCfg *cfg);

void estrutura_inserir(Estrutura *e, ITEM *it);

typedef void (*ITEMVisitor)(ITEM *it, void *ctx);
void estrutura_percorrer(Estrutura *e, ITEMVisitor cb, void *ctx);

size_t estrutura_tamanho(const Estrutura *e);

void estrutura_destroy(Estrutura *e);

void estrutura_set_owns_items(Estrutura *e, bool owns);

#endif