#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/reader.h"
#include "../include/reader_perfil.h"

// datasets que a gente conhece: o --reader escolhe uma dessas entradas
static const ReaderPerfil *const PERFIS[] = {
    &READER_PERFIL_NASA,
    &READER_PERFIL_DETER
};
#define N_PERFIS ((int)(sizeof PERFIS / sizeof PERFIS[0]))

struct Reader {
    const ReaderPerfil *perfil;
    FILE   *arq;
    ESQUEMA *esquema;  // o reader e o dono, quem libera e o reader_destroy
    char   *linha;  // buffer reaproveitado, cresce quando precisa
    size_t  cap;
    long    num_linha;  // linhas fisicas lidas (o cabecalho conta como 1)
    long    ignoradas;
    long    primeira_invalida;
    char    erro[160];
};

// So aqui a gente depende das assinaturas de esquema_criar/item_criar.
// Se o item.h mudar, so esses dois wrappers precisam mudar.
static ESQUEMA *novo_esquema(const char *cabecalho, char delim, char decimal) {
    return esquema_criar(cabecalho, delim, decimal);
}

static ITEM *novo_item(ESQUEMA *e, const char *linha) {
    return item_criar(e, linha);
}

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
 * Le uma linha fisica inteira (de qualquer tamanho) pra r->linha, com o
 * terminador junto. Devolve false no fim do arquivo ou se faltar memoria.
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
            return len > 0;  // EOF: a ultima linha veio sem \n

        size_t lido = strlen(r->linha + len);
        if (lido == 0) {  // tem um '\0' no meio do arquivo
            definir_erro(r, "%s", "byte nulo encontrado (arquivo binario?)");
            return false;
        }
        len += lido;
        if (r->linha[len - 1] == '\n') return true;
        if (len + 1 < r->cap) return true;  // sobrou espaco no buffer, entao foi EOF sem \n

        // o buffer encheu sem achar \n: a linha continua, entao dobra e le mais
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
        // reabrir apagaria o esquema que os itens vivos ainda usam
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
            esquema_apagar(&r->esquema);  // ainda nao existe nenhum item
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

        // linha so com \r\n (tipo linha em branco no fim) nao e registro
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
