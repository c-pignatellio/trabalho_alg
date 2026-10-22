#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include "item.h"

/* ================================================================ ESQUEMA */

struct esquema_ {
    int n;
    char **nomes;
    char delim;
    char decimal;
    char *cabecalho;
    const char *fim;
};

/* strdup/strndup nao sao ISO C99: copia propria. */
static char *copiar(const char *s, size_t n) {
    char *p = malloc(n + 1);
    if (p == NULL) return NULL;

    memcpy(p, s, n);
    p[n] = '\0';

    return p;
}

/* Remove apenas o terminador da linha. */
static size_t tamanho_sem_fim(const char *s, bool *tinha_cr) {
    size_t n = strlen(s);

    *tinha_cr = false;

    if (n > 0 && s[n - 1] == '\n')
        n--;

    if (n > 0 && s[n - 1] == '\r') {
        *tinha_cr = true;
        n--;
    }

    return n;
}

/*
 * Divide uma linha CSV em campos.
 * Trata aspas e aspas duplas escapadas.
 * Retorna -1 se encontrar aspas invalidas.
 */
static int dividir(const char *s, size_t len, char delim,
                   char *buf, int *off, int max_campos) {
    int n = 0;
    size_t pos = 0;
    size_t i = 0;

    bool aspas = false;
    bool fechou_aspas = false;
    bool inicio_campo = true;

    if (max_campos <= 0) return -1;

    off[0] = 0;

    for (; i < len; i++) {
        char c = s[i];

        if (aspas) {
            if (c == '"') {
                if (i + 1 < len && s[i + 1] == '"') {
                    buf[pos++] = '"';
                    i++;
                } else {
                    aspas = false;
                    fechou_aspas = true;
                }
            } else {
                buf[pos++] = c;
            }
        } else if (fechou_aspas) {
            if (c == delim) {
                buf[pos++] = '\0';
                n++;

                if (n < max_campos)
                    off[n] = (int)pos;

                fechou_aspas = false;
                inicio_campo = true;
            } else {
                /* Depois das aspas finais, so pode vir delimitador ou fim. */
                return -1;
            }
        } else if (c == delim) {
            buf[pos++] = '\0';
            n++;

            if (n < max_campos)
                off[n] = (int)pos;

            inicio_campo = true;
        } else if (c == '"') {
            /* Aspas so podem abrir no inicio de um campo. */
            if (!inicio_campo)
                return -1;

            aspas = true;
            inicio_campo = false;
        } else {
            buf[pos++] = c;
            inicio_campo = false;
        }
    }

    /* Linha terminou antes de fechar as aspas. */
    if (aspas)
        return -1;

    buf[pos] = '\0';

    return n + 1;
}

ESQUEMA *esquema_criar(const char *linha_cabecalho,
                       char delimitador, char sep_decimal) {
    if (linha_cabecalho == NULL) return NULL;

    bool cr;
    size_t len = tamanho_sem_fim(linha_cabecalho, &cr);

    ESQUEMA *e = calloc(1, sizeof(ESQUEMA));
    if (e == NULL) return NULL;

    e->delim = delimitador;
    e->decimal = sep_decimal;
    e->fim = cr ? "\r\n" : "\n";
    e->cabecalho = copiar(linha_cabecalho, len);

    /* Maximo de campos = numero de delimitadores + 1. */
    int max = 1;

    for (size_t i = 0; i < len; i++) {
        if (linha_cabecalho[i] == delimitador)
            max++;
    }

    char *buf = malloc(len + (size_t)max + 1);
    int *off = malloc((size_t)max * sizeof(int));
    e->nomes = calloc((size_t)max, sizeof(char *));

    if (e->cabecalho == NULL || buf == NULL ||
        off == NULL || e->nomes == NULL) {
        free(buf);
        free(off);

        e->n = max;
        esquema_apagar(&e);

        return NULL;
    }

    int quantidade = dividir(linha_cabecalho, len,
                             delimitador, buf, off, max);

    if (quantidade <= 0) {
        free(buf);
        free(off);

        e->n = max;
        esquema_apagar(&e);

        return NULL;
    }

    e->n = quantidade;

    for (int i = 0; i < e->n; i++) {
        e->nomes[i] = copiar(buf + off[i],
                             strlen(buf + off[i]));

        if (e->nomes[i] == NULL) {
            free(buf);
            free(off);
            esquema_apagar(&e);

            return NULL;
        }
    }

    free(buf);
    free(off);

    return e;
}

int esquema_num_campos(const ESQUEMA *e) {
    return e ? e->n : 0;
}

const char *esquema_nome_campo(const ESQUEMA *e, int idx) {
    if (e == NULL || idx < 0 || idx >= e->n)
        return "";

    return e->nomes[idx];
}

int esquema_indice(const ESQUEMA *e, const char *nome) {
    if (e == NULL || nome == NULL)
        return -1;

    for (int i = 0; i < e->n; i++) {
        if (e->nomes[i] != NULL &&
            strcmp(e->nomes[i], nome) == 0)
            return i;
    }

    return -1;
}

void esquema_escrever_cabecalho(const ESQUEMA *e, FILE *saida) {
    if (e == NULL || saida == NULL) return;

    fputs(e->cabecalho, saida);
    fputs(e->fim, saida);
}

void esquema_apagar(ESQUEMA **e) {
    if (e == NULL || *e == NULL) return;

    if ((*e)->nomes != NULL) {
        for (int i = 0; i < (*e)->n; i++)
            free((*e)->nomes[i]);

        free((*e)->nomes);
    }

    free((*e)->cabecalho);
    free(*e);

    *e = NULL;
}

/* ================================================================== ITEM */

typedef struct {
    int off;
    bool tem_num;
    double num;
} CAMPO;

struct item_ {
    const ESQUEMA *esq;  /* Nao e dono do esquema. */
    char *linha;
    char *buf;
    CAMPO *campos;
    int refs;
};

static bool igual_ignorando_caixa(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) !=
            tolower((unsigned char)*b))
            return false;

        a++;
        b++;
    }

    return *a == '\0' && *b == '\0';
}

/* Converte o texto de um campo para numero ou booleano. */
static bool interpretar_numero(const char *txt,
                               char decimal, double *out) {
    while (isspace((unsigned char)*txt))
        txt++;

    if (*txt == '\0')
        return false;

    /*
     * Permite espacos no final de true/false.
     */
    size_t tam = strlen(txt);

    while (tam > 0 && isspace((unsigned char)txt[tam - 1]))
        tam--;

    char booleano[6];

    if (tam < sizeof(booleano)) {
        memcpy(booleano, txt, tam);
        booleano[tam] = '\0';

        if (igual_ignorando_caixa(booleano, "true")) {
            *out = 1.0;
            return true;
        }

        if (igual_ignorando_caixa(booleano, "false")) {
            *out = 0.0;
            return true;
        }
    }

    if (!(isdigit((unsigned char)*txt) ||
          *txt == '-' || *txt == '+' ||
          *txt == '.' ||
          (decimal == ',' && *txt == ',')))
        return false;

    char tmp[64];
    size_t n = strlen(txt);

    if (n >= sizeof(tmp))
        return false;

    for (size_t i = 0; i <= n; i++) {
        tmp[i] = (decimal == ',' && txt[i] == ',')
                     ? '.'
                     : txt[i];
    }

    char *fim;

    errno = 0;
    double v = strtod(tmp, &fim);

    if (fim == tmp || errno == ERANGE || !isfinite(v))
        return false;

    while (isspace((unsigned char)*fim))
        fim++;

    if (*fim != '\0')
        return false;

    *out = v;

    return true;
}

ITEM *item_criar(const ESQUEMA *e, const char *linha) {
    if (e == NULL || linha == NULL || e->n <= 0)
        return NULL;

    bool cr;
    size_t len = tamanho_sem_fim(linha, &cr);
    int n = e->n;

    ITEM *it = calloc(1, sizeof(ITEM));
    if (it == NULL) return NULL;

    it->esq = e;
    it->refs = 1;

    it->linha = copiar(linha, len);
    it->buf = malloc(len + (size_t)n + 2);
    it->campos = calloc((size_t)n, sizeof(CAMPO));

    int *off = malloc((size_t)n * sizeof(int));

    if (it->linha == NULL || it->buf == NULL ||
        it->campos == NULL || off == NULL) {
        free(off);
        item_apagar(&it);

        return NULL;
    }

    int achados = dividir(linha, len, e->delim,
                          it->buf, off, n);

    /*
     * Rejeita linhas malformadas ou com quantidade de
     * campos diferente da definida pelo esquema.
     */
    if (achados != n) {
        free(off);
        item_apagar(&it);

        return NULL;
    }

    for (int i = 0; i < n; i++) {
        it->campos[i].off = off[i];
    }

    for (int i = 0; i < n; i++) {
        it->campos[i].tem_num =
            interpretar_numero(
                it->buf + it->campos[i].off,
                e->decimal,
                &it->campos[i].num
            );
    }

    free(off);

    return it;
}

ITEM *item_referenciar(ITEM *item) {
    if (item == NULL)
        return NULL;

    /* Evita overflow do contador de referencias. */
    if (item->refs <= 0 || item->refs == INT_MAX)
        return NULL;

    item->refs++;

    return item;
}

void item_apagar(ITEM **item) {
    if (item == NULL || *item == NULL)
        return;

    ITEM *it = *item;
    *item = NULL;

    if (it->refs > 1) {
        it->refs--;
        return;
    }

    free(it->linha);
    free(it->buf);
    free(it->campos);
    free(it);
}

const ESQUEMA *item_esquema(const ITEM *item) {
    return item ? item->esq : NULL;
}

const char *item_texto(const ITEM *item, int idx) {
    if (item == NULL || idx < 0 || idx >= item->esq->n)
        return "";

    return item->buf + item->campos[idx].off;
}

bool item_ausente(const ITEM *item, int idx) {
    return item_texto(item, idx)[0] == '\0';
}

bool item_numero(const ITEM *item, int idx, double *valor) {
    if (item == NULL || idx < 0 || idx >= item->esq->n)
        return false;

    if (!item->campos[idx].tem_num)
        return false;

    if (valor != NULL)
        *valor = item->campos[idx].num;

    return true;
}

void item_escrever(const ITEM *item, FILE *saida) {
    if (item == NULL || saida == NULL)
        return;

    fputs(item->linha, saida);
    fputs(item->esq->fim, saida);
}