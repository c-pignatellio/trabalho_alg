#include <stdlib.h>
#include "../../include/estruturas/lista_cabeca.h"

typedef struct no_ NO_CABECA;
struct no_ {
    ITEM *item;  // fica NULL no no sentinela
    NO_CABECA *prox;
};

struct lista_cabeca_ {
    NO_CABECA *cabeca;  // sentinela fixo: todo no real tem um anterior, entao nao tem caso especial no inicio
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

bool lista_cabeca_buscar(const LISTA_CABECA *l, const ITEM *item) {
    if (l == NULL) return false;
    for (NO_CABECA *n = l->cabeca->prox; n != NULL; n = n->prox) {
        if (n->item == item) return true;
    }
    return false;
}

ITEM *lista_cabeca_remover(LISTA_CABECA *l, const ITEM *item) {
    if (l == NULL) return NULL;
    NO_CABECA *ant = l->cabeca;
    while (ant->prox != NULL && ant->prox->item != item) ant = ant->prox;
    if (ant->prox == NULL) return NULL;
    NO_CABECA *alvo = ant->prox;
    ITEM *removido = alvo->item;
    ant->prox = alvo->prox;
    free(alvo);
    l->tamanho--;
    return removido;
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