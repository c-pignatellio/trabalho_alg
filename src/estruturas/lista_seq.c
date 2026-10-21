#include <stdlib.h>
#include "../../include/estruturas/lista_seq.h"

struct lista_seq_ {
    ITEM **itens;
    size_t fim;
    size_t capacidade;
};

LISTA_SEQ *lista_seq_criar(size_t capacidade) {
    LISTA_SEQ *l = malloc(sizeof(LISTA_SEQ));
    if (l != NULL) {
        l->capacidade = capacidade > 0 ? capacidade : 100;
        l->fim = 0;
        l->itens = malloc(l->capacidade * sizeof(ITEM *));
        if (l->itens == NULL) { free(l); return NULL; }
    }
    return l;
}

bool lista_seq_inserir(LISTA_SEQ *l, ITEM *item) {
    if (l == NULL) return false;
    if (l->fim >= l->capacidade) {
        size_t nova_cap = l->capacidade * 2;
        ITEM **temp = realloc(l->itens, nova_cap * sizeof(ITEM *));
        if (temp == NULL) return false;
        l->itens = temp;
        l->capacidade = nova_cap;
    }
    l->itens[l->fim++] = item;
    return true;
}

ITEM *lista_seq_obter(const LISTA_SEQ *l, size_t idx) {
    if (l == NULL || idx >= l->fim) return NULL;
    return l->itens[idx];
}

size_t lista_seq_tamanho(const LISTA_SEQ *l) {
    return l ? l->fim : 0;
}

void lista_seq_percorrer(LISTA_SEQ *l, void (*cb)(ITEM*, void*), void *ctx) {
    if (l == NULL || cb == NULL) return;
    for (size_t i = 0; i < l->fim; i++) {
        cb(l->itens[i], ctx);
    }
}

void lista_seq_destruir(LISTA_SEQ *l, bool owns_items) {
    if (l == NULL) return;
    if (owns_items) {
        for (size_t i = 0; i < l->fim; i++) {
            item_apagar(&l->itens[i]);
        }
    }
    free(l->itens);
    free(l);
}