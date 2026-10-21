#include <stdlib.h>
#include "lista_seq.h"

struct lista_seq_ {
    ITEM **itens;
    int fim;
    int capacidade;
};

LISTA_SEQ *lista_seq_criar(int capacidade) {
    LISTA_SEQ *l = malloc(sizeof(LISTA_SEQ));
    if (l != NULL) {
        l->capacidade = capacidade > 0 ? capacidade : 100;
        l->fim = 0;
        l->itens = malloc(l->capacidade * sizeof(ITEM *));
    }
    return l;
}

bool lista_seq_inserir(LISTA_SEQ *l, ITEM *item) {
    if (l == NULL) return false;
    if (l->fim >= l->capacidade) {
        l->capacidade *= 2;
        ITEM **temp = realloc(l->itens, l->capacidade * sizeof(ITEM *));
        if (temp == NULL) return false;
        l->itens = temp;
    }
    l->itens[l->fim++] = item;
    return true;
}

ITEM *lista_seq_obter(const LISTA_SEQ *l, int idx) {
    if (l == NULL || idx < 0 || idx >= l->fim) return NULL;
    return l->itens[idx];
}

int lista_seq_tamanho(const LISTA_SEQ *l) {
    return l ? l->fim : 0;
}

void lista_seq_percorrer(const LISTA_SEQ *l, CallbackSeq funcao, void *contexto) {
    if (l == NULL || funcao == NULL) return;
    for (int i = 0; i < l->fim; i++) {
        funcao(l->itens[i], contexto);
    }
}

void lista_seq_apagar(LISTA_SEQ **l) {
    if (l == NULL || *l == NULL) return;
    for (int i = 0; i < (*l)->fim; i++) {
        item_apagar(&((*l)->itens[i]));
    }
    free((*l)->itens);
    free(*l);
    *l = NULL;
}