#include "../../include/estruturas/estrutura.h"
#include <stdlib.h>
#include <string.h>

#include "../../include/estruturas/pilha.h"
#include "../../include/estruturas/fila.h"
#include "../../include/estruturas/lista_seq.h"
#include "../../include/estruturas/lista_enc.h"
#include "../../include/estruturas/lista_cabeca.h"
#include "../../include/estruturas/lista_ord.h"
#include "../../include/estruturas/lista_gen.h"
#include "../../include/estruturas/lista_cruz.h"
#include "../../include/item.h"

typedef enum {
    T_PILHA, T_FILA, T_SEQ, T_ENC, T_CABECA, T_ORD, T_GEN, T_CRUZ
} Tipo;

typedef struct {
    void (*inserir)(void *impl, ITEM *it, Estrutura *e);
    void (*percorrer)(void *impl, ITEMVisitor cb, void *ctx);
    size_t (*tamanho)(const void *impl);
    void (*destruir)(void *impl, bool owns);
} VTable;

struct Estrutura {
    Tipo    tipo;
    void   *impl;
    const VTable *vt;
    bool    owns_items;

    char   *ordem_campo;
    bool    ordem_desc;

    const char **group;    int n_group;
    const char **cross;    int n_cross;

    size_t  idx_gen;
    size_t  idx_cruz;
};

static int indice_ordem(Estrutura *e, const ITEM *it) {
    if (e == NULL || it == NULL || e->ordem_campo == NULL)
        return -1;

    const ESQUEMA *esq = item_esquema(it);
    if (esq == NULL)
        return -1;

    return esquema_indice(esq, e->ordem_campo);
}

static double chave_numerica(Estrutura *e, const ITEM *it) {
    int idx = indice_ordem(e, it);
    if (idx < 0) return 0.0;
    double v;
    return item_numero(it, idx, &v) ? v : 0.0;
}

static void ord_inserir(void *impl, ITEM *it, Estrutura *e) {
    double k = chave_numerica(e, it);
    if (e->ordem_desc)
        k = -k;
    lista_ord_inserir((LISTA_ORD *)impl, it, k);
}

static void pilha_w_inserir(void *impl, ITEM *it, Estrutura *e) {
    (void)e;
    pilha_inserir((PILHA *)impl, it);
}
static void fila_w_inserir(void *impl, ITEM *it, Estrutura *e) {
    (void)e;
    fila_inserir((FILA *)impl, it);
}
static void seq_inserir(void *impl, ITEM *it, Estrutura *e) {
    (void)e;
    lista_seq_inserir((LISTA_SEQ *)impl, it);
}
static void enc_inserir(void *impl, ITEM *it, Estrutura *e) {
    (void)e;
    lista_enc_inserir((LISTA_ENC *)impl, it);
}
static void cabeca_inserir(void *impl, ITEM *it, Estrutura *e) {
    (void)e;
    lista_cabeca_inserir((LISTA_CABECA *)impl, it);
}
static void gen_inserir(void *impl, ITEM *it, Estrutura *e) {
    Estrutura *em = e;
    const ESQUEMA *esq = item_esquema(it);

    const char *grupo = "default", *sub = "default";
    if (e->n_group >= 1 && e->group) {
        int idx = esquema_indice(esq, e->group[0]);
        if (idx >= 0) grupo = item_texto(it, idx);
    }
    if (e->n_group >= 2 && e->group) {
        int idx = esquema_indice(esq, e->group[1]);
        if (idx >= 0) sub = item_texto(it, idx);
    }
    lista_gen_inserir((LISTA_GEN*)impl, grupo, sub, it);
}

static void cruz_inserir(void *impl, ITEM *it, Estrutura *e) {
    const ESQUEMA *esq = item_esquema(it);
    const char *v1 = "default", *v2 = "default";
    if (e->n_cross >= 1 && e->cross) {
        int idx = esquema_indice(esq, e->cross[0]);
        if (idx >= 0) v1 = item_texto(it, idx);
    }
    if (e->n_cross >= 2 && e->cross) {
        int idx = esquema_indice(esq, e->cross[1]);
        if (idx >= 0) v2 = item_texto(it, idx);
    }
    lista_cruz_inserir((LISTA_CRUZ*)impl, it, v1, v2);
}



/* pilha */
static void   pilha_perc_v(void *impl, ITEMVisitor cb, void *ctx){ pilha_percorrer((PILHA*)impl, cb, ctx); }
static size_t pilha_tam_v (const void *impl){ return pilha_tamanho((const PILHA*)impl); }
static void   pilha_dest_v(void *impl, bool owns){ pilha_destruir((PILHA*)impl, owns); }
static const VTable VT_PILHA = { pilha_w_inserir, pilha_perc_v, pilha_tam_v, pilha_dest_v };

/* fila */
static void   fila_perc_v(void *impl, ITEMVisitor cb, void *ctx){ fila_percorrer((FILA*)impl, cb, ctx); }
static size_t fila_tam_v (const void *impl){ return fila_tamanho((const FILA*)impl); }
static void   fila_dest_v(void *impl, bool owns){ fila_destruir((FILA*)impl, owns); }
static const VTable VT_FILA  = { fila_w_inserir,  fila_perc_v,  fila_tam_v,  fila_dest_v  };

/* seq */
static void   seq_perc_v(void *impl, ITEMVisitor cb, void *ctx){ lista_seq_percorrer((LISTA_SEQ*)impl, cb, ctx); }
static size_t seq_tam_v (const void *impl){ return lista_seq_tamanho((const LISTA_SEQ*)impl); }
static void   seq_dest_v(void *impl, bool owns){ lista_seq_destruir((LISTA_SEQ*)impl, owns); }
static const VTable VT_SEQ = { seq_inserir, seq_perc_v, seq_tam_v, seq_dest_v };

/* cabeca */
static void   cabeca_perc_v(void *impl, ITEMVisitor cb, void *ctx){ lista_cabeca_percorrer((LISTA_CABECA*)impl, cb, ctx); }
static size_t cabeca_tam_v (const void *impl){ return lista_cabeca_tamanho((const LISTA_CABECA*)impl); }
static void   cabeca_dest_v(void *impl, bool owns){ lista_cabeca_destruir((LISTA_CABECA*)impl, owns); }
static const VTable VT_CABECA = { cabeca_inserir, cabeca_perc_v, cabeca_tam_v, cabeca_dest_v };

/* ord */
static void   ord_perc_v(void *impl, ITEMVisitor cb, void *ctx){ lista_ord_percorrer((LISTA_ORD*)impl, cb, ctx); }
static size_t ord_tam_v (const void *impl){ return lista_ord_tamanho((const LISTA_ORD*)impl); }
static void   ord_dest_v(void *impl, bool owns){ lista_ord_destruir((LISTA_ORD*)impl, owns); }
static const VTable VT_ORD = { ord_inserir, ord_perc_v, ord_tam_v, ord_dest_v };

/* gen */
static void   gen_perc_v(void *impl, ITEMVisitor cb, void *ctx){ lista_gen_percorrer((LISTA_GEN*)impl, cb, ctx); }
static size_t gen_tam_v (const void *impl){ return lista_gen_tamanho((const LISTA_GEN*)impl); }
static void   gen_dest_v(void *impl, bool owns){ lista_gen_destruir((LISTA_GEN*)impl, owns); }
static const VTable VT_GEN = { gen_inserir, gen_perc_v, gen_tam_v, gen_dest_v };

static void   cruz_perc_v(void *impl, ITEMVisitor cb, void *ctx){ lista_cruz_percorrer((LISTA_CRUZ*)impl, cb, ctx); }
static size_t cruz_tam_v (const void *impl){ return lista_cruz_tamanho((const LISTA_CRUZ*)impl); }
static void   cruz_dest_v(void *impl, bool owns){ lista_cruz_destruir((LISTA_CRUZ*)impl, owns); }
static const VTable VT_CRUZ = { cruz_inserir, cruz_perc_v, cruz_tam_v, cruz_dest_v };




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

    if (cfg) {
        if (cfg->ordem_campo) {
            size_t n = strlen(cfg->ordem_campo);
            e->ordem_campo = malloc(n + 1);
            if (!e->ordem_campo) { free(e); return NULL; }
            memcpy(e->ordem_campo, cfg->ordem_campo, n + 1);
            if (!e->ordem_campo) { free(e); return NULL; }
        }
        e->ordem_desc = cfg->ordem_desc;
        e->group      = cfg->group;
        e->n_group    = cfg->n_group;
        e->cross      = cfg->cross;
        e->n_cross    = cfg->n_cross;
    }

    if      (!strcmp(nome, "pilha"))        { e->tipo = T_PILHA;  e->vt = &VT_PILHA;  e->impl = pilha_criar(); }
    else if (!strcmp(nome, "fila"))         { e->tipo = T_FILA;   e->vt = &VT_FILA;   e->impl = fila_criar(); }
    else if (!strcmp(nome, "lista_seq"))    { e->tipo = T_SEQ;    e->vt = &VT_SEQ;    e->impl = lista_seq_criar(0); }
    else if (!strcmp(nome, "lista_enc"))    { e->tipo = T_ENC;    e->vt = &VT_ENC;    e->impl = lista_enc_criar(); }
    else if (!strcmp(nome, "lista_cabeca")) { e->tipo = T_CABECA; e->vt = &VT_CABECA; e->impl = lista_cabeca_criar(); }
    else if (!strcmp(nome, "lista_ord"))    { e->tipo = T_ORD;    e->vt = &VT_ORD;    e->impl = lista_ord_criar(); }
    else if (!strcmp(nome, "lista_gen"))    { e->tipo = T_GEN;    e->vt = &VT_GEN;    e->impl = lista_gen_criar(); }
    else if (!strcmp(nome, "lista_cruz"))   { e->tipo = T_CRUZ;   e->vt = &VT_CRUZ;   e->impl = lista_cruz_criar(); }
    else { free(e); return NULL; }

    if (!e->impl) { free(e->ordem_campo); free(e); return NULL; }
    e->owns_items = true;
    return e;
}

void estrutura_inserir(Estrutura *e, ITEM *it) {
    e->vt->inserir(e->impl, it, e);
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
    free(e->ordem_campo);
    free(e);
}