#include <stdlib.h>
#include "../../include/estruturas/lista_enc.h"

typedef struct no_ NO_ENC;
struct no_ {
    ITEM *item;
    NO_ENC *prox;
};

struct lista_enc_ {
    NO_ENC *inicio;
    size_t tamanho;
};

LISTA_ENC *lista_enc_criar(void) {
    LISTA_ENC *l = malloc(sizeof(LISTA_ENC));
    if (l != NULL) {
        l->inicio = NULL;
        l->tamanho = 0;
    }
    return l;
}

bool lista_enc_inserir(LISTA_ENC *l, ITEM *item) {
    if (l == NULL) return false;
    NO_ENC *novo = malloc(sizeof(NO_ENC));
    if (novo == NULL) return false;
    novo->item = item;
    novo->prox = l->inicio;
    l->inicio = novo;
    l->tamanho++;
    return true;
}

size_t lista_enc_tamanho(const LISTA_ENC *l) {
    return l ? l->tamanho : 0;
}

void lista_enc_percorrer(LISTA_ENC *l, void (*cb)(ITEM*, void*), void *ctx) {
    if (l == NULL || cb == NULL) return;
    for (NO_ENC *n = l->inicio; n != NULL; n = n->prox) {
        cb(n->item, ctx);
    }
}

void lista_enc_destruir(LISTA_ENC *l, bool owns_items) {
    if (l == NULL) return;
    NO_ENC *n = l->inicio;
    while (n != NULL) {
        NO_ENC *prox = n->prox;
        if (owns_items && n->item != NULL) {
            item_apagar(&n->item);
        }
        free(n);
        n = prox;
    }
    free(l);
}