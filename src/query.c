#include "../include/query.h"
#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

/* ================================================================ TIPOS */

typedef enum { OP_EQ, OP_NE, OP_LE, OP_GE, OP_LT, OP_GT } Op;

typedef struct {
    char campo[64];
    Op   op;
    bool alvo_num;
    double alvo;
    char   alvo_txt[64];
} Filter;

typedef struct {
    char   campo[64];
    double alvo;
    double tol;
    double peso;
} Sim;

#define MAX_LISTA 2   /* GROUP/CROSS: no maximo 2 dimensoes (limite dos TADs) */

struct Query {
    Filter *filters; int n_filters, cap_filters;
    Sim    *sims;    int n_sims,    cap_sims;

    bool   tem_accept;
    double accept;

    char   order_campo[64];
    bool   order_desc;

    char  *group[MAX_LISTA]; int n_group;
    char  *cross[MAX_LISTA]; int n_cross;

    /* Cache da ultima avaliacao (usado por ORDER|similarity). */
    const ITEM *ultimo_item;
    double      ultima_sim;
};

/* ============================================================ UTILIDADES */

static char *copiar(const char *s, size_t n) {
    char *p = malloc(n + 1);
    if (p) { memcpy(p, s, n); p[n] = '\0'; }
    return p;
}

static void trim(char *s) {
    if (!s) return;

    char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);

    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) s[--n] = '\0';
}

/*
 * Divide por '|' no proprio buffer (escreve '\0' nos separadores).
 * Devolve a quantidade de tokens (>= 1). Tolerante a tokens vazios.
 */
static int dividir_pipe(char *linha, char *toks[], int max) {
    if (max <= 0) return 0;

    int n = 0;
    toks[n++] = linha;
    for (char *p = linha; *p && n < max; p++) {
        if (*p == '|') { *p = '\0'; toks[n++] = p + 1; }
    }
    return n;
}

/* Interpreta alvo numerico (ponto decimal). Aceita +/-, expoente. */
static bool parse_num(const char *s, double *out) {
    if (!s || !*s) return false;

    errno = 0;
    char *fim = NULL;
    double v = strtod(s, &fim);
    if (fim == s || errno == ERANGE || !isfinite(v)) return false;

    while (*fim && isspace((unsigned char)*fim)) fim++;
    if (*fim != '\0') return false;

    *out = v;
    return true;
}

/* `true`/`false` case-insensitive -> 1/0. Caso contrario, false. */
static bool parse_bool_txt(const char *s, double *out) {
    if (!s) return false;
    while (*s && isspace((unsigned char)*s)) s++;
    if (!strcasecmp(s, "true"))  { *out = 1.0; return true; }
    if (!strcasecmp(s, "false")) { *out = 0.0; return true; }
    return false;
}

static bool parse_op(const char *s, Op *op) {
    if (!s) return false;
    if (!strcmp(s, "==")) { *op = OP_EQ; return true; }
    if (!strcmp(s, "!=")) { *op = OP_NE; return true; }
    if (!strcmp(s, "<=")) { *op = OP_LE; return true; }
    if (!strcmp(s, ">=")) { *op = OP_GE; return true; }
    if (!strcmp(s, "<"))  { *op = OP_LT; return true; }
    if (!strcmp(s, ">"))  { *op = OP_GT; return true; }
    return false;
}

static bool cmp_num(double v, Op op, double alvo) {
    switch (op) {
        case OP_EQ: return v == alvo;
        case OP_NE: return v != alvo;
        case OP_LE: return v <= alvo;
        case OP_GE: return v >= alvo;
        case OP_LT: return v <  alvo;
        case OP_GT: return v >  alvo;
    }
    return false;
}

/* Comparacao de texto case-insensitive; ordem lexicografica em tolower. */
static int cmp_txt_ci(const char *a, const char *b) {
    while (*a && *b) {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

static bool cmp_txt(const char *v, Op op, const char *alvo) {
    int r = cmp_txt_ci(v, alvo);
    switch (op) {
        case OP_EQ: return r == 0;
        case OP_NE: return r != 0;
        case OP_LE: return r <= 0;
        case OP_GE: return r >= 0;
        case OP_LT: return r <  0;
        case OP_GT: return r >  0;
    }
    return false;
}

/* Verifica se um campo referenciado existe no cabecalho do item.
 * Como o ESQUEMA e unico por Reader, validar aqui so confirma presenca
 * de nome; nao ha custo relevante (poucas colunas). */
static int idx_campo(const ITEM *it, const char *nome) {
    const ESQUEMA *e = item_esquema(it);
    if (!e || !nome) return -1;
    return esquema_indice(e, nome);
}

/* ============================================================ CARGA */

static void falha(const char *linha_fisica, long n, const char *motivo) {
    fprintf(stderr, "query: linha %ld: %s\n  %s\n",
            n, motivo, linha_fisica ? linha_fisica : "");
}

static bool push_filter(Query *q, const Filter *f) {
    if (q->n_filters >= q->cap_filters) {
        int nc = q->cap_filters ? q->cap_filters * 2 : 8;
        Filter *n = realloc(q->filters, (size_t)nc * sizeof *n);
        if (!n) return false;
        q->filters = n;
        q->cap_filters = nc;
    }
    q->filters[q->n_filters++] = *f;
    return true;
}

static bool push_sim(Query *q, const Sim *s) {
    if (q->n_sims >= q->cap_sims) {
        int nc = q->cap_sims ? q->cap_sims * 2 : 4;
        Sim *n = realloc(q->sims, (size_t)nc * sizeof *n);
        if (!n) return false;
        q->sims = n;
        q->cap_sims = nc;
    }
    q->sims[q->n_sims++] = *s;
    return true;
}

Query *query_load(const char *caminho) {
    if (!caminho) return NULL;

    FILE *f = fopen(caminho, "r");
    if (!f) { perror("query: fopen"); return NULL; }

    Query *q = calloc(1, sizeof *q);
    if (!q) { fclose(f); return NULL; }
    q->ultima_sim = 1.0;

    char linha[512];
    long n = 0;
    bool ok = true;

    while (ok && fgets(linha, sizeof linha, f)) {
        n++;
        size_t len = strlen(linha);
        while (len > 0 && (linha[len - 1] == '\n' || linha[len - 1] == '\r'))
            linha[--len] = '\0';
        trim(linha);

        if (len == 0 || linha[0] == '#') continue;

        /* Guardar copia para o relatorio de erro (trim mexe no buffer). */
        char original[512];
        strncpy(original, linha, sizeof original - 1);
        original[sizeof original - 1] = '\0';

        char *tok[8];
        int nt = dividir_pipe(linha, tok, 8);

        if (nt < 1) { falha(original, n, "linha vazia apos split"); ok = false; break; }

        if (!strcmp(tok[0], "FILTER")) {
            if (nt < 4) { falha(original, n, "FILTER espera 4 campos"); ok = false; break; }

            Filter flt = {0};
            if (strlen(tok[1]) >= sizeof flt.campo) {
                falha(original, n, "nome de campo longo demais"); ok = false; break;
            }
            strcpy(flt.campo, tok[1]);

            if (!parse_op(tok[2], &flt.op)) {
                falha(original, n, "operador invalido"); ok = false; break;
            }

            double vnum;
            if (parse_num(tok[3], &vnum)) {
                flt.alvo_num = true;
                flt.alvo = vnum;
            } else {
                /* Alvo textual: aceita true/false e strings genericas. */
                if (strlen(tok[3]) >= sizeof flt.alvo_txt) {
                    falha(original, n, "alvo textual longo demais"); ok = false; break;
                }
                strcpy(flt.alvo_txt, tok[3]);
                flt.alvo_num = false;

                /* Operadores relacionais so fazem sentido para numeros. */
                if (flt.op != OP_EQ && flt.op != OP_NE) {
                    falha(original, n, "operador relacional exige alvo numerico");
                    ok = false; break;
                }
            }

            if (!push_filter(q, &flt)) { falha(original, n, "sem memoria"); ok = false; }

        } else if (!strcmp(tok[0], "SIM")) {
            if (nt < 5) { falha(original, n, "SIM espera 5 campos"); ok = false; break; }

            Sim s = {0};
            if (strlen(tok[1]) >= sizeof s.campo) {
                falha(original, n, "nome de campo longo demais"); ok = false; break;
            }
            strcpy(s.campo, tok[1]);

            if (!parse_num(tok[2], &s.alvo) ||
                !parse_num(tok[3], &s.tol)  ||
                !parse_num(tok[4], &s.peso)) {
                falha(original, n, "SIM requer numeros em alvo/tol/peso");
                ok = false; break;
            }
            if (s.tol <= 0.0) {
                falha(original, n, "tolerancia deve ser > 0"); ok = false; break;
            }
            if (s.peso < 0.0) {
                falha(original, n, "peso nao pode ser negativo"); ok = false; break;
            }
            if (!push_sim(q, &s)) { falha(original, n, "sem memoria"); ok = false; }

        } else if (!strcmp(tok[0], "ACCEPT")) {
            if (nt < 2) { falha(original, n, "ACCEPT espera 2 campos"); ok = false; break; }
            double v;
            if (!parse_num(tok[1], &v)) {
                falha(original, n, "ACCEPT requer numero"); ok = false; break;
            }
            q->tem_accept = true;
            q->accept = v;

        } else if (!strcmp(tok[0], "ORDER")) {
            if (nt < 3) { falha(original, n, "ORDER espera 3 campos"); ok = false; break; }
            if (strlen(tok[1]) >= sizeof q->order_campo) {
                falha(original, n, "nome de campo longo demais"); ok = false; break;
            }
            strcpy(q->order_campo, tok[1]);
            if (!strcasecmp(tok[2], "DESC"))      q->order_desc = true;
            else if (!strcasecmp(tok[2], "ASC"))  q->order_desc = false;
            else { falha(original, n, "ORDER: use ASC ou DESC"); ok = false; break; }

        } else if (!strcmp(tok[0], "GROUP") || !strcmp(tok[0], "CROSS")) {
            bool eh_group = (tok[0][0] == 'G');
            if (nt < 2) { falha(original, n, "GROUP/CROSS exige ao menos 1 campo"); ok = false; break; }
            if (nt - 1 > MAX_LISTA) {
                falha(original, n, "GROUP/CROSS: no maximo 2 campos");
                ok = false; break;
            }
            for (int i = 1; i < nt; i++) {
                char *copia = copiar(tok[i], strlen(tok[i]));
                if (!copia) { falha(original, n, "sem memoria"); ok = false; break; }
                if (eh_group) q->group[q->n_group++] = copia;
                else          q->cross[q->n_cross++] = copia;
            }

        } else {
            falha(original, n, "comando desconhecido"); ok = false; break;
        }
    }

    if (ok && ferror(f)) {
        fprintf(stderr, "query: erro de leitura em '%s'\n", caminho);
        ok = false;
    }
    fclose(f);

    if (!ok) { query_destroy(q); return NULL; }

    /* ORDER|similarity so faz sentido com ao menos um SIM. */
    if (q->order_campo[0] && !strcasecmp(q->order_campo, "similarity") &&
        q->n_sims == 0) {
        fprintf(stderr, "query: ORDER|similarity sem SIM\n");
        query_destroy(q);
        return NULL;
    }

    return q;
}

/* ========================================================== AVALIACAO */

bool query_accepts(const Query *q, const ITEM *it) {
    if (!q || !it) return false;
    const ESQUEMA *esq = item_esquema(it);
    if (!esq) return false;

    /* 1) FILTERs (AND). */
    for (int i = 0; i < q->n_filters; i++) {
        const Filter *f = &q->filters[i];
        int idx = idx_campo(it, f->campo);
        if (idx < 0) return false;

        if (f->alvo_num) {
            double v;
            if (!item_numero(it, idx, &v)) return false;
            if (!cmp_num(v, f->op, f->alvo)) return false;
        } else {
            const char *txt = item_texto(it, idx);
            if (!cmp_txt(txt, f->op, f->alvo_txt)) return false;
        }
    }

    /* 2) Similaridade (se houver SIM). */
    double sim = 1.0;
    if (q->n_sims > 0) {
        double soma = 0.0, soma_peso = 0.0;
        for (int i = 0; i < q->n_sims; i++) {
            const Sim *s = &q->sims[i];
            int idx = idx_campo(it, s->campo);
            if (idx < 0) return false;

            double v;
            if (!item_numero(it, idx, &v)) return false;

            double d = fabs(v - s->alvo);
            double num = 1.0 - d / s->tol;
            if (num < 0.0) num = 0.0;

            soma      += num * s->peso;
            soma_peso += s->peso;
        }
        sim = (soma_peso > 0.0) ? (soma / soma_peso) : 0.0;
    }

    if (q->tem_accept && q->n_sims > 0 && sim < q->accept) return false;

    /* 3) Cache para ORDER|similarity. */
    ((Query *)q)->ultimo_item = it;
    ((Query *)q)->ultima_sim  = sim;
    return true;
}

double query_last_similarity(const Query *q) {
    if (!q) return 1.0;
    return q->ultima_sim;
}

/* ========================================================== ACESSORES */

const char *query_order_field(const Query *q) {
    if (!q || q->order_campo[0] == '\0') return NULL;
    return q->order_campo;
}

bool query_order_desc(const Query *q) {
    return q ? q->order_desc : false;
}

const char *const *query_group_fields(const Query *q) {
    return q ? (const char *const *)q->group : NULL;
}
int query_group_nfields(const Query *q) {
    return q ? q->n_group : 0;
}

const char *const *query_cross_fields(const Query *q) {
    return q ? (const char *const *)q->cross : NULL;
}
int query_cross_nfields(const Query *q) {
    return q ? q->n_cross : 0;
}

/* ============================================================ LIMPEZA */

void query_destroy(Query *q) {
    if (!q) return;

    for (int i = 0; i < q->n_group; i++) free(q->group[i]);
    for (int i = 0; i < q->n_cross; i++) free(q->cross[i]);

    free(q->filters);
    free(q->sims);
    free(q);
}