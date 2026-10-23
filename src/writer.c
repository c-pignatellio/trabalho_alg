#include "../include/writer.h"

#include <stdio.h>
#include <stdlib.h>

struct Writer {
    FILE *arq;
    bool  falhou;
};

Writer *writer_create(const char *caminho, const ESQUEMA *esq) {
    if (!caminho || !esq) return NULL;

    Writer *w = calloc(1, sizeof *w);
    if (!w) return NULL;

    /* "wb": sem traducao de fim de linha (o terminador ja vem do ESQUEMA). */
    w->arq = fopen(caminho, "wb");
    if (!w->arq) { free(w); return NULL; }

    esquema_escrever_cabecalho(esq, w->arq);
    if (ferror(w->arq)) {
        fclose(w->arq);
        free(w);
        return NULL;
    }

    return w;
}

bool writer_write(Writer *w, const ITEM *it) {
    if (!w || !w->arq || w->falhou || !it) return false;

    item_escrever(it, w->arq);

    if (ferror(w->arq)) { w->falhou = true; return false; }
    return true;
}

void writer_destroy(Writer *w) {
    if (!w) return;
    if (w->arq) {
        if (fclose(w->arq) != 0) {
            fprintf(stderr, "writer: erro ao fechar saida\n");
        }
    }
    free(w);
}