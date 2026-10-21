#include <stdlib.h>
#include "../../include/estruturas/lista_cabeca.h"

typedef struct no_ NO_CABECA;
struct no_ {
    ITEM *item; // NULL na cabeça sentinela
    NO_CABECA *prox;
};

struct lista_cabeca_ {
    NO_CABECA *cabeca;
    size_t tamanho;
};

LISTA_CABECA *lista_cabeca_criar(void) {
    LISTA_CABECA *l = malloc(sizeof(LISTA_CABECA));
    if (l != NULL) {
        l->cabeca = malloc(sizeof(NO_CABECA));
        if (l->cabeca == NULL) { free(l); return NULL; }
        l->cabeca->item = NULL;
        l->cabeca->prox = NULL;
        l->tamanho = 0;
    }
    return l;
}

bool lista_cabeca_inserir(LISTA_CABECA *l, ITEM *item) {
    if (l == NULL) return false;
    NO_CABECA *novo = malloc(sizeof(NO_CABECA));
    if (novo == NULL) return false;
    novo->item = item;
    novo->prox = l->cabeca->prox;
    l->cabeca->prox = novo;
    l->tamanho++;
    return true;
}

size_t lista_cabeca_tamanho(const LISTA_CABECA *l) {
    return l ? l->tamanho : 0;
}

void lista_cabeca_percorrer(LISTA_CABECA *l, void (*cb)(ITEM*, void*), void *ctx) {
    if (l == NULL || cb == NULL) return;
    for (NO_CABECA *n = l->cabeca->prox; n != NULL; n = n->prox) {
        cb(n->item, ctx);
    }
}

void lista_cabeca_destruir(LISTA_CABECA *l, bool owns_items) {
    if (l == NULL) return;
    NO_CABECA *n = l->cabeca;
    while (n != NULL) {
        NO_CABECA *prox = n->prox;
        if (owns_items && n->item != NULL) {
            item_apagar(&n->item);
        }
        free(n);
        n = prox;
    }
    free(l);
}