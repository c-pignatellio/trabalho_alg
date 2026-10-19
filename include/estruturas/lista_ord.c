#include <stdlib.h>
#include "lista_ord.h"

typedef struct no_ NO_ORD;
struct no_ {
    ITEM *item;
    double chave_ordenacao;
    NO_ORD *prox;
};

struct lista_ord_ {
    NO_ORD *inicio;
    int tamanho;
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
    if (ant == NULL) {
        l->inicio = novo;
    } else {
        ant->prox = novo;
    }
    l->tamanho++;
    return true;
}

int lista_ord_tamanho(const LISTA_ORD *l) {
    return l ? l->tamanho : 0;
}

void lista_ord_percorrer(const LISTA_ORD *l, CallbackOrd funcao, void *contexto) {
    if (l == NULL || funcao == NULL) return;
    NO_ORD *atual = l->inicio;
    while (atual != NULL) {
        funcao(atual->item, contexto);
        atual = atual->prox;
    }
}

void lista_ord_apagar(LISTA_ORD **l) {
    if (l == NULL || *l == NULL) return;
    NO_ORD *atual = (*l)->inicio;
    while (atual != NULL) {
        NO_ORD *prox = atual->prox;
        item_apagar(&(atual->item));
        free(atual);
        atual = prox;
    }
    free(*l);
    *l = NULL;
}