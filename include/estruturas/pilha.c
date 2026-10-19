#include <stdlib.h>
#include "pilha.h"

typedef struct no_ NO_PILHA;
struct no_ {
    ITEM *item;
    NO_PILHA *prox;
};

struct pilha_ {
    NO_PILHA *topo;
    int tamanho;
};

PILHA *pilha_criar(void) {
    PILHA *p = malloc(sizeof(PILHA));
    if (p != NULL) {
        p->topo = NULL;
        p->tamanho = 0;
    }
    return p;
}

bool pilha_empilhar(PILHA *p, ITEM *item) {
    if (p == NULL) return false;
    NO_PILHA *novo = malloc(sizeof(NO_PILHA));
    if (novo == NULL) return false;
    novo->item = item;
    novo->prox = p->topo;
    p->topo = novo;
    p->tamanho++;
    return true;
}

ITEM *pilha_desempilhar(PILHA *p) {
    if (pilha_vazia(p)) return NULL;
    NO_PILHA *aux = p->topo;
    ITEM *item = aux->item;
    p->topo = aux->prox;
    free(aux);
    p->tamanho--;
    return item;
}

ITEM *pilha_topo(const PILHA *p) {
    return pilha_vazia(p) ? NULL : p->topo->item;
}

bool pilha_vazia(const PILHA *p) {
    return (p == NULL || p->topo == NULL);
}

int pilha_tamanho(const PILHA *p) {
    return p ? p->tamanho : 0;
}

void pilha_apagar(PILHA **p) {
    if (p == NULL || *p == NULL) return;
    while (!pilha_vazia(*p)) {
        ITEM *it = pilha_desempilhar(*p);
        item_apagar(&it);
    }
    free(*p);
    *p = NULL;
}