#include <stdlib.h>
#include <string.h>
#include "../../include/estruturas/lista_gen.h"

typedef struct subgrupo_ SUBGRUPO;
typedef struct grupo_ GRUPO;

struct subgrupo_ {
    char nome[64];
    ITEM **itens;
    size_t qtd;
    size_t capacidade;
    SUBGRUPO *prox;
};

struct grupo_ {
    char nome[64];
    SUBGRUPO *subgrupos;
    GRUPO *prox;
};

struct lista_gen_ {
    GRUPO *grupos;
    size_t total;
};

LISTA_GEN *lista_gen_criar(void) {
    LISTA_GEN *lg = malloc(sizeof(LISTA_GEN));
    if (lg != NULL) { lg->grupos = NULL; lg->total = 0; }
    return lg;
}

bool lista_gen_inserir(LISTA_GEN *lg, const char *nome_grupo, const char *nome_subgrupo, ITEM *item) {
    if (lg == NULL) return false;

    GRUPO *g = lg->grupos, *ant_g = NULL;
    while (g != NULL && strcmp(g->nome, nome_grupo) != 0) { ant_g = g; g = g->prox; }
    if (g == NULL) {
        g = calloc(1, sizeof(GRUPO));
        if (!g) return false;
        strncpy(g->nome, nome_grupo, sizeof(g->nome) - 1);
        if (ant_g == NULL) lg->grupos = g; else ant_g->prox = g;
    }

    SUBGRUPO *s = g->subgrupos, *ant_s = NULL;
    while (s != NULL && strcmp(s->nome, nome_subgrupo) != 0) { ant_s = s; s = s->prox; }
    if (s == NULL) {
        s = calloc(1, sizeof(SUBGRUPO));
        if (!s) return false;
        strncpy(s->nome, nome_subgrupo, sizeof(s->nome) - 1);
        s->capacidade = 10;
        s->itens = malloc(s->capacidade * sizeof(ITEM *));
        if (!s->itens) { free(s); return false; }
        if (ant_s == NULL) g->subgrupos = s; else ant_s->prox = s;
    }

    if (s->qtd >= s->capacidade) {
        size_t nova_cap = s->capacidade * 2;
        ITEM **temp = realloc(s->itens, nova_cap * sizeof(ITEM *));
        if (temp == NULL) return false;
        s->itens = temp;
        s->capacidade = nova_cap;
    }
    s->itens[s->qtd++] = item;
    lg->total++;
    return true;
}

size_t lista_gen_tamanho(const LISTA_GEN *lg) {
    return lg ? lg->total : 0;
}

void lista_gen_percorrer(LISTA_GEN *lg, void (*cb)(ITEM*, void*), void *ctx) {
    if (lg == NULL || cb == NULL) return;
    for (GRUPO *g = lg->grupos; g != NULL; g = g->prox) {
        for (SUBGRUPO *s = g->subgrupos; s != NULL; s = s->prox) {
            for (size_t i = 0; i < s->qtd; i++) cb(s->itens[i], ctx);
        }
    }
}

void lista_gen_destruir(LISTA_GEN *lg, bool owns_items) {
    if (lg == NULL) return;
    GRUPO *g = lg->grupos;
    while (g != NULL) {
        SUBGRUPO *s = g->subgrupos;
        while (s != NULL) {
            SUBGRUPO *prox_s = s->prox;
            if (owns_items) {
                for (size_t i = 0; i < s->qtd; i++) item_apagar(&s->itens[i]);
            }
            free(s->itens);
            free(s);
            s = prox_s;
        }
        GRUPO *prox_g = g->prox;
        free(g);
        g = prox_g;
    }
    free(lg);
}