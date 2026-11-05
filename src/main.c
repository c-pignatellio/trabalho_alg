#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <time.h>

#include "../include/estruturas/item.h"
#include "../include/reader.h"
#include "../include/writer.h"
#include "../include/query.h"
#include "../include/estruturas/estrutura.h"

// tempo atual em milissegundos (relogio monotonico)
static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

// argumentos da linha de comando
typedef struct {
    const char *reader, *input, *base, *result, *query, *output;
    long top_k, limit;
} Args;

/* Le os argumentos da linha de comando e preenche o Args.
   Devolve true se todos os obrigatorios vieram. */
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

typedef struct {
    const Query *q;  // a consulta em uso
    Estrutura   *result;  // onde os aceitos vao sendo inseridos
    long         matched;  // quantos passaram na consulta
    double       insert_ms;  // soma do tempo gasto inserindo em result
} BuscaCtx;  // contexto que vai pro callback da busca

static void busca_visita(ITEM *it, void *ctx_) {
    BuscaCtx *ctx = ctx_;
    if (!query_accepts(ctx->q, it)) return;
    ctx->matched++;
    double t0 = now_ms();
    estrutura_inserir(ctx->result, it);
    ctx->insert_ms += now_ms() - t0;
}

typedef struct {
    Writer *w;  // quem escreve a saida
    long    top_k;  // maximo de itens (0 = todos)
    long    written;  // quantos ja foram escritos
    bool    stop;  // bateu o top-k: ignora o resto (o percurso em si nao para)
} WriteCtx;

// callback do percurso do resultado: escreve os itens ate bater o top-k
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

    do {
        // reader: cria o leitor do dataset escolhido e abre o arquivo
        rd = reader_create(a.reader);
        if (!rd || !reader_open(rd, a.input)) {
            fprintf(stderr, "erro: reader '%s' ou input '%s'\n",
                    a.reader, a.input);
            break;
        }

        // writer: ja escreve o cabecalho assim que e criado
        wr = writer_create(a.output, reader_esquema(rd));
        if (!wr) {
            fprintf(stderr, "erro: output '%s'\n", a.output);
            break;
        }

        // query: le FILTER/SIM/ACCEPT (a busca) e ORDER/GROUP/CROSS (como organizar o resultado)
        q = query_load(a.query);
        if (!q) {
            fprintf(stderr, "erro: query '%s'\n", a.query);
            break;
        }

        // config que as duas estruturas compartilham
        EstruturaCfg cfg = {
            .ordem_campo = query_order_field(q),
            .ordem_desc  = query_order_desc(q),
            .group       = query_group_fields(q),
            .n_group     = query_group_nfields(q),
            .cross       = query_cross_fields(q),
            .n_cross     = query_cross_nfields(q),
            .query       = q,
        };

        // base: cria a estrutura escolhida e deixa ela como dona dos itens (libera tudo no destroy)
        base = estrutura_create(a.base, &cfg);
        if (!base) {
            fprintf(stderr, "erro: base '%s'\n", a.base);
            break;
        }
        estrutura_set_owns_items(base, true);

        // LOAD: le todas as linhas do dataset e joga na base
        double t_load0 = now_ms();
        long input_records = 0;
        ITEM *it;
        while ((it = reader_next(rd)) != NULL) {
            if (a.limit > 0 && input_records >= a.limit) {
                item_apagar(&it);
                break;
            }
            estrutura_inserir(base, it);
            input_records++;
        }
        double load_ms = now_ms() - t_load0;

        // result: so guarda ponteiros emprestados, quem e dona dos itens e a base
        res = estrutura_create(a.result, &cfg);
        if (!res) { fprintf(stderr, "erro: result '%s'\n", a.result); break; }
        estrutura_set_owns_items(res, false);

        // SEARCH + RESULT_INSERT: percorre a base aplicando a consulta e insere os aceitos na result
        BuscaCtx bctx = { .q = q, .result = res, .matched = 0, .insert_ms = 0 };
        double t_search0 = now_ms();
        estrutura_percorrer(base, busca_visita, &bctx);
        double search_ms = now_ms() - t_search0;

        // WRITE: percorre a result na ordem natural dela e escreve ate o top-k
        WriteCtx wctx = { .w = wr, .top_k = a.top_k, .written = 0, .stop = false };
        double t_write0 = now_ms();
        estrutura_percorrer(res, write_visita, &wctx);
        double write_ms = now_ms() - t_write0;

        // resumo na saida padrao
        printf("STATUS=OK\n");
        printf("INPUT_RECORDS=%ld\n",  input_records);
        printf("MATCHED_RECORDS=%ld\n", bctx.matched);
        printf("OUTPUT_RECORDS=%ld\n",  wctx.written);
        printf("LOAD_MS=%.3f\n",          load_ms);
        printf("SEARCH_MS=%.3f\n",        search_ms);
        printf("RESULT_INSERT_MS=%.3f\n", bctx.insert_ms);
        printf("WRITE_MS=%.3f\n",         write_ms);

        rc = 0;
    } while (0);

    // limpeza
    estrutura_destroy(res);
    estrutura_destroy(base);
    query_destroy(q);
    
    if (rd) {
        reader_close(rd);
        reader_destroy(rd);
    }

    if (wr) writer_destroy(wr);

    return rc;
}