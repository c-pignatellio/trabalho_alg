#ifndef WRITER_H
#define WRITER_H

#include <stdbool.h>
#include "estruturas/item.h"

/*
 * TAD Writer: grava o arquivo de saida igualzinho ao de entrada (mesmo
 * cabecalho, colunas, ordem, delimitador e terminador de linha).
 *
 * O writer_create ja escreve o cabecalho na hora, a partir do ESQUEMA do
 * Reader. Depois disso o Writer nao depende mais nem do ESQUEMA nem do
 * Reader, entao pode continuar vivo depois do reader_close/destroy.
 *
 * Uso mais comum:
 *   Writer *w = writer_create(out, reader_esquema(rd));
 *   writer_write(w, it);   // quantas vezes precisar
 *   writer_destroy(w);
 *
 * Se uma escrita falhar, o Writer fica marcado como defeituoso e as
 * proximas chamadas ja devolvem false sem tentar de novo. Depois do
 * primeiro false, a saida nao serve mais.
 */

typedef struct Writer Writer;

// Abre o arquivo e escreve o cabecalho. Devolve NULL se `caminho` ou `esq`
// forem invalidos ou se der erro de I/O nessa parte.
Writer *writer_create(const char *caminho, const ESQUEMA *esq);

// grava `it` e o terminador de linha original do dataset
bool writer_write(Writer *w, const ITEM *it);

// fecha e libera; pode passar NULL
void writer_destroy(Writer *w);

#endif