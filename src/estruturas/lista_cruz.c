#include <stdlib.h>
#include <string.h>
#include "../../include/estruturas/lista_cruz.h"

typedef struct no_cruz_ NO_CRUZ;
typedef struct cabeca_dim_ CABECA_DIM;

struct no_cruz_ {
    ITEM *item;
    NO_CRUZ *prox_dim1;
    NO_CRUZ *prox_dim2;
};

struct cabeca_dim_ {
    char valor[64];
    NO_CRUZ *primeiro_no;
    CABECA_DIM *prox;
};

struct lista_cruz_ {
    CABECA_DIM *dim1_cabecas;
    CABECA_DIM *dim2_cabecas;
    size_t total;
};

LISTA_CRUZ *lista_cruz_criar(void) {
    LISTA_CRUZ *lc = malloc(sizeof(LISTA_CRUZ));
    if (lc != NULL) {
        lc->dim1_cabecas = NULL;
        lc->dim2_cabecas = NULL;
        lc->total = 0;
    }
    return lc;
}

static CABECA_DIM *obter_ou_criar_cabeca(CABECA_DIM **lista, const char *valor) {
    CABECA_DIM *atual = *lista, *ant = NULL;
    while (atual != NULL && strcmp(atual->valor, valor) != 0) { ant = atual; atual = atual->prox; }
    if (atual == NULL) {
        atual = calloc(1, sizeof(CABECA_DIM));
        if (!atual) return NULL;
        strncpy(atual->valor, valor, sizeof(atual->valor) - 1);
        if (ant == NULL) *lista = atual; else ant->prox = atual;
    }
    return atual;
}

bool lista_cruz_inserir(LISTA_CRUZ *lc, ITEM *item, const char *dim1_val, const char *dim2_val) {
    if (lc == NULL) return false;

    NO_CRUZ *novo = malloc(sizeof(NO_CRUZ));
    if (novo == NULL) return false;
    novo->item = item;
    novo->prox_dim1 = NULL;
    novo->prox_dim2 = NULL;

    CABECA_DIM *c1 = obter_ou_criar_cabeca(&lc->dim1_cabecas, dim1_val);
    if (!c1) { free(novo); return false; }

    CABECA_DIM *c2 = obter_ou_criar_cabeca(&lc->dim2_cabecas, dim2_val);
    if (!c2) { free(novo); return false; }   /* c1 não foi mutada ainda → OK */

    novo->prox_dim1 = c1->primeiro_no;
    c1->primeiro_no = novo;
    novo->prox_dim2 = c2->primeiro_no;
    c2->primeiro_no = novo;

    lc->total++;
    return true;
}

size_t lista_cruz_tamanho(const LISTA_CRUZ *lc) {
    return lc ? lc->total : 0;
}

void lista_cruz_percorrer(LISTA_CRUZ *lc, void (*cb)(ITEM*, void*), void *ctx) {
    if (lc == NULL || cb == NULL) return;
    for (CABECA_DIM *c = lc->dim1_cabecas; c != NULL; c = c->prox) {
        for (NO_CRUZ *n = c->primeiro_no; n != NULL; n = n->prox_dim1) {
            cb(n->item, ctx);
        }
    }
}

void lista_cruz_destruir(LISTA_CRUZ *lc, bool owns_items) {
    if (lc == NULL) return;

    CABECA_DIM *c1 = lc->dim1_cabecas;
    while (c1 != NULL) {
        NO_CRUZ *n = c1->primeiro_no;
        while (n != NULL) {
            NO_CRUZ *prox = n->prox_dim1;
            if (owns_items && n->item != NULL) item_apagar(&n->item);
            free(n);
            n = prox;
        }
        CABECA_DIM *prox_c = c1->prox;
        free(c1);
        c1 = prox_c;
    }

    CABECA_DIM *c2 = lc->dim2_cabecas;
    while (c2 != NULL) {
        CABECA_DIM *prox_c = c2->prox;
        free(c2);
        c2 = prox_c;
    }

    free(lc);
}