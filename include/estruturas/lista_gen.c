#include <stdlib.h>
#include <string.h>
#include "lista_gen.h"

typedef struct subgrupo_ SUBGRUPO;
typedef struct grupo_ GRUPO;

struct subgrupo_ {
    char nome[64];
    ITEM **itens;
    int qtd;
    int capacidade;
    SUBGRUPO *prox;
};

struct grupo_ {
    char nome[64];
    SUBGRUPO *subgrupos;
    GRUPO *prox;
};

struct lista_gen_ {
    GRUPO *grupos;
};

LISTA_GEN *lista_gen_criar(void) {
    LISTA_GEN *lg = malloc(sizeof(LISTA_GEN));
    if (lg != NULL) lg->grupos = NULL;
    return lg;
}

bool lista_gen_adicionar(LISTA_GEN *lg, const char *nome_grupo, const char *nome_subgrupo, ITEM *item) {
    if (lg == NULL) return false;

    GRUPO *g = lg->grupos;
    GRUPO *ant_g = NULL;
    while (g != NULL && strcmp(g->nome, nome_grupo) != 0) {
        ant_g = g;
        g = g->prox;
    }
    if (g == NULL) {
        g = malloc(sizeof(GRUPO));
        strncpy(g->nome, nome_grupo, sizeof(g->nome) - 1);
        g->subgrupos = NULL;
        g->prox = NULL;
        if (ant_g == NULL) lg->grupos = g;
        else ant_g->prox = g;
    }

    SUBGRUPO *s = g->subgrupos;
    SUBGRUPO *ant_s = NULL;
    while (s != NULL && strcmp(s->nome, nome_subgrupo) != 0) {
        ant_s = s;
        s = s->prox;
    }
    if (s == NULL) {
        s = malloc(sizeof(SUBGRUPO));
        strncpy(s->nome, nome_subgrupo, sizeof(s->nome) - 1);
        s->capacidade = 10;
        s->qtd = 0;
        s->itens = malloc(s->capacidade * sizeof(ITEM *));
        s->prox = NULL;
        if (ant_s == NULL) g->subgrupos = s;
        else ant_s->prox = s;
    }

    if (s->qtd >= s->capacidade) {
        s->capacidade *= 2;
        ITEM **temp = realloc(s->itens, s->capacidade * sizeof(ITEM *));
        if (temp == NULL) return false;
        s->itens = temp;
    }
    s->itens[s->qtd++] = item;
    return true;
}

void lista_gen_apagar(LISTA_GEN **lg) {
    if (lg == NULL || *lg == NULL) return;
    GRUPO *g = (*lg)->grupos;
    while (g != NULL) {
        SUBGRUPO *s = g->subgrupos;
        while (s != NULL) {
            SUBGRUPO *prox_s = s->prox;
            for (int i = 0; i < s->qtd; i++) {
                item_apagar(&(s->itens[i]));
            }
            free(s->itens);
            free(s);
            s = prox_s;
        }
        GRUPO *prox_g = g->prox;
        free(g);
        g = prox_g;
    }
    free(*lg);
    *lg = NULL;
}