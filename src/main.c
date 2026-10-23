/*

----COMENTARIOS SOBRE A MAIN:----

FIZERAM PRA MIM NUM CONTEXTO GERAL DO PROJETO JA CONSIDERANDO READER, WRITER, QUERY E AS FUNCOES TUDO
MAS QUANDO FORMOS DESENVOLVER ESSES ARQUIVOS PODE SER QUE MUDE COISA DA MAIN
O QUE RECOMENDO É DEIXAR POR ENQUANDO PRA TER MEIO COMO CONSULTA MAS HONESTAMENTE PODE APAGAR TUDO QUE EU N VOU LIGAR
(SÓ AS ROTAS DOS ARQUIVOS QUE PRECISA PRESTAR ATENCAO)

*/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <time.h>

#include "./item.h"
#include "../include/estruturas/reader.h"
#include "../include/estruturas/writer.h"
#include "../include/estruturas/query.h"
#include "../include/estruturas/estrutura.h"

/* ---------- utilidades ---------- */

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

typedef struct {
    const char *reader, *input, *base, *result, *query, *output;
    long top_k, limit;
} Args;

static bool parse_args(int argc, char **argv, Args *a) {
    memset(a, 0, sizeof *a);
    static struct option opts[] = {
        {"reader", required_argument, 0, 'r'},
        {"input",  required_argument, 0, 'i'},
        {"base",   required_argument, 0, 'b'},
        {"result", required_argument, 0, 'R'},
        {"query",  required_argument, 0, 'q'},
        {"output", required_argument, 0, 'o'},
        {"top-k",  required_argument, 0, 'k'},
        {"limit",  required_argument, 0, 'l'},
        {0,0,0,0}
    };
    int c;
    while ((c = getopt_long(argc, argv, "", opts, NULL)) != -1) {
        switch (c) {
            case 'r': a->reader = optarg; break;
            case 'i': a->input  = optarg; break;
            case 'b': a->base   = optarg; break;
            case 'R': a->result = optarg; break;
            case 'q': a->query  = optarg; break;
            case 'o': a->output = optarg; break;
            case 'k': a->top_k  = strtol(optarg, NULL, 10); break;
            case 'l': a->limit  = strtol(optarg, NULL, 10); break;
            default: return false;
        }
    }
    return a->reader && a->input && a->base && a->result
        && a->query && a->output;
}

/* ---------- contexto da busca (passa pelo callback) ---------- */

typedef struct {
    const Query *q;
    Estrutura   *result;
    long         matched;
    double       insert_ms;   /* acumulado só do insert em result */
} BuscaCtx;

static void busca_visita(ITEM *it, void *ctx_) {
    BuscaCtx *ctx = ctx_;
    if (!query_accepts(ctx->q, it)) return;
    ctx->matched++;
    double t0 = now_ms();
    estrutura_inserir(ctx->result, it);   /* posse emprestada: base é dona */
    ctx->insert_ms += now_ms() - t0;
}

/* ---------- contexto da escrita ---------- */

typedef struct {
    Writer *w;
    long    top_k;      /* 0 = todos */
    long    written;
    bool    stop;
} WriteCtx;

static void write_visita(ITEM *it, void *ctx_) {
    WriteCtx *ctx = ctx_;
    if (ctx->stop) return;
    if (ctx->top_k > 0 && ctx->written >= ctx->top_k) {
        ctx->stop = true;
        return;
    }
    writer_write(ctx->w, it);
    ctx->written++;
}

/* ---------- main ---------- */

int main(int argc, char **argv) {
    Args a;
    if (!parse_args(argc, argv, &a)) {
        fprintf(stderr, "uso: ./trabalho --reader N --input F --base B "
                        "--result R --query Q --output O "
                        "[--top-k K] [--limit L]\n");
        return 2;
    }

    int rc = 1;
    Reader   *rd   = NULL;
    Writer   *wr   = NULL;
    Estrutura*base = NULL;
    Estrutura*res  = NULL;
    Query    *q    = NULL;

    /* 1. reader */
    rd = reader_create(a.reader);
    if (!rd || !reader_open(rd, a.input)) {
        fprintf(stderr, "erro: reader '%s' ou input '%s'\n",
                a.reader, a.input);
        goto fim;
    }

    /* 2. writer (cabeçalho já sai) */
    wr = writer_create(a.output, reader_esquema(rd));
    if (!wr) { fprintf(stderr, "erro: output '%s'\n", a.output); goto fim; }

    /* 3. query (metadados p/ estruturas avançadas) */
    q = query_load(a.query);
    if (!q) { fprintf(stderr, "erro: query '%s'\n", a.query); goto fim; }

    /* 4. cfg comum às duas estruturas */
    EstruturaCfg cfg = {
    .ordem_campo = query_order_field(q),
    .ordem_desc  = query_order_desc(q),
    .group       = query_group_fields(q),
    .n_group     = query_group_nfields(q),
    .cross       = query_cross_fields(q),
    .n_cross     = query_cross_nfields(q),
    .query       = q,
};

    /* 5. base */
    base = estrutura_create(a.base, &cfg);
    if (!base) { fprintf(stderr, "erro: base '%s'\n", a.base); goto fim; }
    estrutura_set_owns_items(base, true);

    /* 6. LOAD */
    double t_load0 = now_ms();
    long input_records = 0;
    ITEM *it;
    while ((it = reader_next(rd)) != NULL) {
        if (a.limit > 0 && input_records >= a.limit) { item_free(it); break; }
        estrutura_inserir(base, it);
        input_records++;
    }
    double load_ms = now_ms() - t_load0;

    /* 7. result (ponteiros emprestados: base é dona) */
    res = estrutura_create(a.result, &cfg);
    if (!res) { fprintf(stderr, "erro: result '%s'\n", a.result); goto fim; }
    estrutura_set_owns_items(res, false);

    /* 8. SEARCH + RESULT_INSERT */
    BuscaCtx bctx = { .q = q, .result = res, .matched = 0, .insert_ms = 0 };
    double t_search0 = now_ms();
    estrutura_percorrer(base, busca_visita, &bctx);
    double search_ms = now_ms() - t_search0;

    /* 9. WRITE (top-k via percurso natural da result) */
    WriteCtx wctx = { .w = wr, .top_k = a.top_k, .written = 0, .stop = false };
    double t_write0 = now_ms();
    estrutura_percorrer(res, write_visita, &wctx);
    double write_ms = now_ms() - t_write0;

    /* 10. stdout */
    printf("STATUS=OK\n");
    printf("INPUT_RECORDS=%ld\n",  input_records);
    printf("MATCHED_RECORDS=%ld\n", bctx.matched);
    printf("OUTPUT_RECORDS=%ld\n",  wctx.written);
    printf("LOAD_MS=%.3f\n",          load_ms);
    printf("SEARCH_MS=%.3f\n",        search_ms);
    printf("RESULT_INSERT_MS=%.3f\n", bctx.insert_ms);
    printf("WRITE_MS=%.3f\n",         write_ms);

    rc = 0;

fim:
    /* ordem inversa; res não libera ITEMs, base libera */
    estrutura_destroy(res);
    estrutura_destroy(base);
    query_destroy(q);
    if (wr) writer_destroy(wr);
    if (rd) { reader_close(rd); reader_destroy(rd); }
    return rc;
}