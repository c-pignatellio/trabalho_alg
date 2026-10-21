#include "../include/estruturas/estrutura.h"
#include <stdlib.h>
#include <string.h>

#include "../include/estruturas/pilha.h"
#include "../include/estruturas/fila.h"
#include "../include/estruturas/lista_seq.h"
#include "../include/estruturas/lista_enc.h"
#include "../include/estruturas/lista_cabeca.h"
#include "../include/estruturas/lista_ord.h"
#include "../include/estruturas/lista_gen.h"
#include "../include/estruturas/lista_cruz.h"

typedef enum {
    T_PILHA, T_FILA, T_SEQ, T_ENC, T_CABECA, T_ORD, T_GEN, T_CRUZ
} Tipo;

typedef struct {
    void (*inserir)(void *impl, ITEM *it);
    void (*percorrer)(void *impl, ITEMVisitor cb, void *ctx);
    size_t (*tamanho)(const void *impl);
    void (*destruir)(void *impl, bool owns);
} VTable;

struct Estrutura {
    Tipo    tipo;
    void   *impl;
    const VTable *vt;
    bool    owns_items;
};

static void enc_inserir(void *impl, ITEM *it) {
    lista_enc_inserir((LISTA_ENC *)impl, it);
}
static void enc_percorrer(void *impl, ITEMVisitor cb, void *ctx) {
    lista_enc_percorrer((LISTA_ENC *)impl, (void (*)(ITEM*, void*))cb, ctx);
}
static size_t enc_tamanho(const void *impl) {
    return lista_enc_tamanho((const LISTA_ENC *)impl);
}
static void enc_destruir(void *impl, bool owns) {
    lista_enc_destruir((LISTA_ENC *)impl, owns);
}

static const VTable VT_ENC = {
    enc_inserir, enc_percorrer, enc_tamanho, enc_destruir
};

Estrutura *estrutura_create(const char *nome, const EstruturaCfg *cfg) {
    Estrutura *e = calloc(1, sizeof *e);
    if (!e) return NULL;

    if (strcmp(nome, "pilha") == 0) {
        e->tipo = T_PILHA; e->vt = &VT_PILHA;
        e->impl = pilha_criar();
    } else if (strcmp(nome, "fila") == 0) {
        e->tipo = T_FILA; e->vt = &VT_FILA;
        e->impl = fila_criar();
    } else if (strcmp(nome, "lista_seq") == 0) {
        e->tipo = T_SEQ; e->vt = &VT_SEQ;
        e->impl = lista_seq_criar();
    } else if (strcmp(nome, "lista_enc") == 0) {
        e->tipo = T_ENC; e->vt = &VT_ENC;
        e->impl = lista_enc_criar();
    } else if (strcmp(nome, "lista_cabeca") == 0) {
        e->tipo = T_CABECA; e->vt = &VT_CABECA;
        e->impl = lista_cabeca_criar();
    } else if (strcmp(nome, "lista_ord") == 0) {
        e->tipo = T_ORD; e->vt = &VT_ORD;
        e->impl = lista_ord_criar(cfg ? cfg->ordem_campo : NULL,
                                  cfg ? cfg->ordem_desc : false);
    } else if (strcmp(nome, "lista_gen") == 0) {
        e->tipo = T_GEN; e->vt = &VT_GEN;
        e->impl = lista_gen_criar(cfg ? cfg->group : NULL,
                                  cfg ? cfg->n_group : 0);
    } else if (strcmp(nome, "lista_cruz") == 0) {
        e->tipo = T_CRUZ; e->vt = &VT_CRUZ;
        e->impl = lista_cruz_criar(cfg ? cfg->cross : NULL,
                                   cfg ? cfg->n_cross : 0);
    } else {
        free(e); return NULL;
    }

    if (!e->impl) { free(e); return NULL; }
    e->owns_items = true;
    return e;
}

void estrutura_inserir(Estrutura *e, ITEM *it) {
    e->vt->inserir(e->impl, it);
}
void estrutura_percorrer(Estrutura *e, ITEMVisitor cb, void *ctx) {
    e->vt->percorrer(e->impl, cb, ctx);
}
size_t estrutura_tamanho(const Estrutura *e) {
    return e->vt->tamanho(e->impl);
}
void estrutura_set_owns_items(Estrutura *e, bool owns) {
    e->owns_items = owns;
}
void estrutura_destroy(Estrutura *e) {
    if (!e) return;
    e->vt->destruir(e->impl, e->owns_items);
    free(e);
}