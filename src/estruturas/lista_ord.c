#include <stdlib.h>
#include "../../include/estruturas/lista_ord.h"

typedef struct no_ NO_ORD;
struct no_ {
    ITEM *item;
    double chave_ordenacao;
    NO_ORD *prox;
};

struct lista_ord_ {
    NO_ORD *inicio;
    size_t tamanho;
};

LISTA_ORD *lista_ord_criar(void) {
    LISTA_ORD *l = malloc(sizeof(LISTA_ORD));
    if (l != NULL) {
        l->inicio = NULL;
        l->tamanho = 0;
    }
    return l;
}

bool lista_ord_inserir(LISTA_ORD *l, ITEM *item, double chave_ordenacao) {
    if (l == NULL) return false;
    NO_ORD *novo = malloc(sizeof(NO_ORD));
    if (novo == NULL) return false;
    novo->item = item;
    novo->chave_ordenacao = chave_ordenacao;

    NO_ORD *ant = NULL;
    NO_ORD *atual = l->inicio;
    while (atual != NULL && atual->chave_ordenacao < chave_ordenacao) {
        ant = atual;
        atual = atual->prox;
    }
    novo->prox = atual;
    if (ant == NULL) l->inicio = novo;
    else ant->prox = novo;
    l->tamanho++;
    return true;
}

size_t lista_ord_tamanho(const LISTA_ORD *l) {
    return l ? l->tamanho : 0;
}

void lista_ord_percorrer(LISTA_ORD *l, void (*cb)(ITEM*, void*), void *ctx) {
    if (l == NULL || cb == NULL) return;
    for (NO_ORD *n = l->inicio; n != NULL; n = n->prox) {
        cb(n->item, ctx);
    }
}

void lista_ord_destruir(LISTA_ORD *l, bool owns_items) {
    if (l == NULL) return;
    NO_ORD *n = l->inicio;
    while (n != NULL) {
        NO_ORD *prox = n->prox;
        if (owns_items && n->item != NULL) {
            item_apagar(&n->item);
        }
        free(n);
        n = prox;
    }
    free(l);
}