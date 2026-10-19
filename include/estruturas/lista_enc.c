#include <stdlib.h>
#include "lista_enc.h"

typedef struct no_ NO_ENC;
struct no_ {
    ITEM *item;
    NO_ENC *prox;
};

struct lista_enc_ {
    NO_ENC *inicio;
    int tamanho;
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

int lista_enc_tamanho(const LISTA_ENC *l) {
    return l ? l->tamanho : 0;
}

void lista_enc_percorrer(const LISTA_ENC *l, CallbackEnc funcao, void *contexto) {
    if (l == NULL || funcao == NULL) return;
    NO_ENC *atual = l->inicio;
    while (atual != NULL) {
        funcao(atual->item, contexto);
        atual = atual->prox;
    }
}

void lista_enc_apagar(LISTA_ENC **l) {
    if (l == NULL || *l == NULL) return;
    NO_ENC *atual = (*l)->inicio;
    while (atual != NULL) {
        NO_ENC *prox = atual->prox;
        item_apagar(&(atual->item));
        free(atual);
        atual = prox;
    }
    free(*l);
    *l = NULL;
}