#include <stdlib.h>
#include "fila.h"

typedef struct no_ NO_FILA;
struct no_ {
    ITEM *item;
    NO_FILA *prox;
};

struct fila_ {
    NO_FILA *inicio;
    NO_FILA *fim;
    int tamanho;
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

bool fila_enfileirar(FILA *f, ITEM *item) {
    if (f == NULL) return false;
    NO_FILA *novo = malloc(sizeof(NO_FILA));
    if (novo == NULL) return false;
    novo->item = item;
    novo->prox = NULL;
    if (f->fim == NULL) {
        f->inicio = novo;
    } else {
        f->fim->prox = novo;
    }
    f->fim = novo;
    f->tamanho++;
    return true;
}

ITEM *fila_desenfileirar(FILA *f) {
    if (fila_vazia(f)) return NULL;
    NO_FILA *aux = f->inicio;
    ITEM *item = aux->item;
    f->inicio = aux->prox;
    if (f->inicio == NULL) {
        f->fim = NULL;
    }
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

int fila_tamanho(const FILA *f) {
    return f ? f->tamanho : 0;
}

void fila_apagar(FILA **f) {
    if (f == NULL || *f == NULL) return;
    while (!fila_vazia(*f)) {
        ITEM *it = fila_desenfileirar(*f);
        item_apagar(&it);
    }
    free(*f);
    *f = NULL;
}