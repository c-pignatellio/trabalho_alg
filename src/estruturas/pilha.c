#include <stdlib.h>
#include "../../include/estruturas/pilha.h"

typedef struct no_ NO_PILHA;
struct no_ {
    ITEM *item;
    NO_PILHA *prox;
};

struct pilha_ {
    NO_PILHA *topo;
    size_t tamanho;
};

PILHA *pilha_criar(void) {
    PILHA *p = malloc(sizeof(PILHA));
    if (p != NULL) {
        p->topo = NULL;
        p->tamanho = 0;
    }
    return p;
}

bool pilha_inserir(PILHA *p, ITEM *item) {
    if (p == NULL) return false;
    NO_PILHA *novo = malloc(sizeof(NO_PILHA));
    if (novo == NULL) return false;
    novo->item = item;
    novo->prox = p->topo;
    p->topo = novo;
    p->tamanho++;
    return true;
}

ITEM *pilha_remover(PILHA *p) {
    if (pilha_vazia(p)) return NULL;
    NO_PILHA *aux = p->topo;
    ITEM *item = aux->item;
    p->topo = aux->prox;
    free(aux);
    p->tamanho--;
    return item;
}

ITEM *pilha_topo(const PILHA *p) {
    return pilha_vazia(p) ? NULL : p->topo->item;
}

bool pilha_vazia(const PILHA *p) {
    return (p == NULL || p->topo == NULL);
}

size_t pilha_tamanho(const PILHA *p) {
    return p ? p->tamanho : 0;
}

void pilha_percorrer(PILHA *p, void (*cb)(ITEM*, void*), void *ctx) {
    if (p == NULL || cb == NULL) return;
    for (NO_PILHA *n = p->topo; n != NULL; n = n->prox) {
        cb(n->item, ctx);
    }
}

void pilha_destruir(PILHA *p, bool owns_items) {
    if (p == NULL) return;
    NO_PILHA *n = p->topo;
    while (n != NULL) {
        NO_PILHA *prox = n->prox;
        if (owns_items && n->item != NULL) {
            item_apagar(&n->item);
        }
        free(n);
        n = prox;
    }
    free(p);
}