#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/reader.h"
#include "../include/reader_perfil.h"

/* Tabela de datasets conhecidos: --reader <nome> escolhe uma entrada. */
static const ReaderPerfil *const PERFIS[] = {
    &READER_PERFIL_NASA,
    &READER_PERFIL_DETER
};
#define N_PERFIS ((int)(sizeof PERFIS / sizeof PERFIS[0]))

struct Reader {
    const ReaderPerfil *perfil;
    FILE   *arq;
    ESQUEMA *esquema;          /* dono: liberado em reader_destroy */
    char   *linha;             /* buffer reutilizado, cresce sob demanda */
    size_t  cap;
    long    num_linha;         /* linhas fisicas lidas (cabecalho = 1) */
    long    ignoradas;
    long    primeira_invalida;
    char    erro[160];
};

/* ---------------------------------------------------------------------
 * Unico ponto que depende das assinaturas de esquema_criar/item_criar.
 * Se item.h mudar (ex.: passar a devolver ITEM_ERRO), so isto muda.
 * ------------------------------------------------------------------- */
static ESQUEMA *novo_esquema(const char *cabecalho, char delim, char decimal) {
    return esquema_criar(cabecalho, delim, decimal);
}

static ITEM *novo_item(ESQUEMA *e, const char *linha) {
    return item_criar(e, linha);
}

/* ------------------------------------------------------------------ */

static void definir_erro(Reader *r, const char *fmt, const char *arg) {
    snprintf(r->erro, sizeof r->erro, fmt, arg);
}

const char *reader_nome(int indice) {
    if (indice < 0 || indice >= N_PERFIS) return NULL;
    return PERFIS[indice]->nome;
}

Reader *reader_create(const char *nome) {
    if (nome == NULL) return NULL;
    for (int i = 0; i < N_PERFIS; i++) {
        if (strcmp(PERFIS[i]->nome, nome) == 0) {
            Reader *r = calloc(1, sizeof *r);
            if (r != NULL) r->perfil = PERFIS[i];
            return r;
        }
    }
    return NULL;
}

/*
 * Le uma linha fisica inteira (qualquer tamanho) para r->linha, incluindo o
 * terminador. Devolve false no fim do arquivo ou em falta de memoria.
 */
static bool ler_linha(Reader *r) {
    if (r->cap == 0) {
        r->linha = malloc(512);
        if (r->linha == NULL) {
            definir_erro(r, "%s", "memoria insuficiente");
            return false;
        }
        r->cap = 512;
    }

    size_t len = 0;
    r->linha[0] = '\0';
    for (;;) {
        if (fgets(r->linha + len, (int)(r->cap - len), r->arq) == NULL)
            return len > 0;                 /* EOF: ultima linha sem \n */

        size_t lido = strlen(r->linha + len);
        if (lido == 0) {                    /* byte '\0' no meio do arquivo */
            definir_erro(r, "%s", "byte nulo encontrado (arquivo binario?)");
            return false;
        }
        len += lido;
        if (r->linha[len - 1] == '\n') return true;
        if (len + 1 < r->cap) return true;  /* buffer sobrou: EOF sem \n */

        /* Buffer cheio sem achar \n: a linha continua, dobra e le mais. */
        char *novo = realloc(r->linha, r->cap * 2);
        if (novo == NULL) {
            definir_erro(r, "%s", "memoria insuficiente");
            return false;
        }
        r->linha = novo;
        r->cap *= 2;
    }
}

bool reader_open(Reader *r, const char *caminho) {
    if (r == NULL) return false;
    r->erro[0] = '\0';

    if (caminho == NULL) {
        definir_erro(r, "%s", "caminho nulo");
        return false;
    }
    if (r->arq != NULL || r->esquema != NULL) {
        /* Reabrir apagaria o esquema que itens ainda vivos usam. */
        definir_erro(r, "%s", "reader ja foi aberto");
        return false;
    }

    r->arq = fopen(caminho, "rb");
    if (r->arq == NULL) {
        definir_erro(r, "nao foi possivel abrir '%s'", caminho);
        return false;
    }

    if (!ler_linha(r)) {
        if (r->erro[0] == '\0') definir_erro(r, "%s", "arquivo vazio");
        reader_close(r);
        return false;
    }
    r->num_linha = 1;

    r->esquema = novo_esquema(r->linha, r->perfil->delimitador,
                              r->perfil->sep_decimal);
    if (r->esquema == NULL) {
        definir_erro(r, "%s", "cabecalho malformado");
        reader_close(r);
        return false;
    }

    for (int i = 0; i < r->perfil->n_colunas_obrigatorias; i++) {
        const char *col = r->perfil->colunas_obrigatorias[i];
        if (esquema_indice(r->esquema, col) < 0) {
            definir_erro(r, "coluna obrigatoria ausente: %s", col);
            esquema_apagar(&r->esquema);   /* nenhum item existe ainda */
            reader_close(r);
            return false;
        }
    }
    return true;
}

const ESQUEMA *reader_esquema(const Reader *r) {
    return r ? r->esquema : NULL;
}

ITEM *reader_next(Reader *r) {
    if (r == NULL || r->arq == NULL || r->esquema == NULL) return NULL;

    while (ler_linha(r)) {
        r->num_linha++;

        /* Linha so com \r\n (ex.: linha em branco no fim): nao e registro. */
        if (r->linha[strspn(r->linha, "\r\n")] == '\0') continue;

        ITEM *it = novo_item(r->esquema, r->linha);
        if (it != NULL) return it;

        r->ignoradas++;
        if (r->primeira_invalida == 0) r->primeira_invalida = r->num_linha;
    }

    if (ferror(r->arq)) definir_erro(r, "%s", "falha de leitura no arquivo");
    return NULL;
}

long reader_linhas_ignoradas(const Reader *r) {
    return r ? r->ignoradas : 0;
}

long reader_primeira_linha_invalida(const Reader *r) {
    return r ? r->primeira_invalida : 0;
}

const char *reader_erro(const Reader *r) {
    return r ? r->erro : "";
}

void reader_close(Reader *r) {
    if (r == NULL || r->arq == NULL) return;
    fclose(r->arq);
    r->arq = NULL;
}

void reader_destroy(Reader *r) {
    if (r == NULL) return;
    reader_close(r);
    esquema_apagar(&r->esquema);
    free(r->linha);
    free(r);
}
