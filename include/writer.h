#ifndef WRITER_H
#define WRITER_H

#include <stdbool.h>
#include "estruturas/item.h"

/*
 * TAD Writer: grava o arquivo de saida no MESMO formato do arquivo de
 * entrada (mesmo cabecalho, mesmas colunas, mesma ordem, mesmo
 * delimitador, mesmo terminador de linha).
 *
 * writer_create escreve o cabecalho imediatamente (a partir do ESQUEMA
 * do Reader); depois disso o Writer NAO depende mais do ESQUEMA nem do
 * Reader, e pode sobreviver a reader_close/destroy.
 *
 * Uso tipico:
 *   Writer *w = writer_create(out, reader_esquema(rd));
 *   ...
 *   writer_write(w, it);
 *   ...
 *   writer_destroy(w);
 *
 * writer_write devolve false se houve falha de I/O (a primeira falha
 * marca o Writer como defeituoso; chamadas seguintes retornam false
 * sem tentar escrever de novo). Apos qualquer false, considere a saida
 * invalida.
 */

typedef struct Writer Writer;

/* Abre o arquivo e escreve o cabecalho. NULL se `caminho` ou `esq`
 * forem invalidos, ou em erro de I/O ao abrir/escrever o cabecalho. */
Writer *writer_create(const char *caminho, const ESQUEMA *esq);

/* Grava `it` seguido do terminador original do dataset. */
bool writer_write(Writer *w, const ITEM *it);

/* Fecha e libera. Seguro com NULL. */
void writer_destroy(Writer *w);

#endif