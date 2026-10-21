#include <stdlib.h>
#include "../../include/estruturas/fila.h"

typedef struct no_ NO_FILA;
struct no_ {
    ITEM *item;
    NO_FILA *prox;
};

struct fila_ {
    NO_FILA *inicio;
    NO_FILA *fim;
    size_t tamanho;
};

FILA *fila_criar(void) {
    FILA *f = malloc(sizeof(FILA));
    if (f != NULL) {
        f->inicio = NULL;
        f->fim = NULL;
        f->tamanho = 0;
    }
    return f;
}

bool fila_inserir(FILA *f, ITEM *item) {
    if (f == NULL) return false;
    NO_FILA *novo = malloc(sizeof(NO_FILA));
    if (novo == NULL) return false;
    novo->item = item;
    novo->prox = NULL;
    if (f->fim == NULL) f->inicio = novo;
    else f->fim->prox = novo;
    f->fim = novo;
    f->tamanho++;
    return true;
}

ITEM *fila_remover(FILA *f) {
    if (fila_vazia(f)) return NULL;
    NO_FILA *aux = f->inicio;
    ITEM *item = aux->item;
    f->inicio = aux->prox;
    if (f->inicio == NULL) f->fim = NULL;
    free(aux);
    f->tamanho--;
    return item;
}

ITEM *fila_frente(const FILA *f) {
    return fila_vazia(f) ? NULL : f->inicio->item;
}

bool fila_vazia(const FILA *f) {
    return (f == NULL || f->inicio == NULL);
}

size_t fila_tamanho(const FILA *f) {
    return f ? f->tamanho : 0;
}

void fila_percorrer(FILA *f, void (*cb)(ITEM*, void*), void *ctx) {
    if (f == NULL || cb == NULL) return;
    for (NO_FILA *n = f->inicio; n != NULL; n = n->prox) {
        cb(n->item, ctx);
    }
}

void fila_destruir(FILA *f, bool owns_items) {
    if (f == NULL) return;
    NO_FILA *n = f->inicio;
    while (n != NULL) {
        NO_FILA *prox = n->prox;
        if (owns_items && n->item != NULL) {
            item_apagar(&n->item);
        }
        free(n);
        n = prox;
    }
    free(f);
}