#include <stdlib.h>
#include "lista_cabeca.h"

typedef struct no_ NO_CABECA;
struct no_ {
    ITEM *item; // NULL na cabeça sentinela
    NO_CABECA *prox;
};

struct lista_cabeca_ {
    NO_CABECA *cabeca;
    int tamanho;
};

LISTA_CABECA *lista_cabeca_criar(void) {
    LISTA_CABECA *l = malloc(sizeof(LISTA_CABECA));
    if (l != NULL) {
        l->cabeca = malloc(sizeof(NO_CABECA));
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

int lista_cabeca_tamanho(const LISTA_CABECA *l) {
    return l ? l->tamanho : 0;
}

void lista_cabeca_percorrer(const LISTA_CABECA *l, CallbackCabeca funcao, void *contexto) {
    if (l == NULL || funcao == NULL) return;
    NO_CABECA *atual = l->cabeca->prox;
    while (atual != NULL) {
        funcao(atual->item, contexto);
        atual = atual->prox;
    }
}

void lista_cabeca_apagar(LISTA_CABECA **l) {
    if (l == NULL || *l == NULL) return;
    NO_CABECA *atual = (*l)->cabeca;
    while (atual != NULL) {
        NO_CABECA *prox = atual->prox;
        if (atual->item != NULL) {
            item_apagar(&(atual->item));
        }
        free(atual);
        atual = prox;
    }
    free(*l);
    *l = NULL;
}