#ifndef READER_PERFIL_H
#define READER_PERFIL_H

/*
 * Perfil de um dataset: tudo o que o Reader precisa saber sobre o formato
 * do arquivo. Uso interno dos leitores (reader.c e reader_<dataset>.c); o
 * restante do programa so enxerga reader.h.
 *
 * Para adicionar um dataset (ex.: "compras"):
 *   1. criar src/reader_compras.c definindo READER_PERFIL_COMPRAS;
 *   2. declarar o extern abaixo;
 *   3. incluir o perfil na tabela PERFIS de reader.c.
 */

typedef struct {
    const char *nome;                        /* valor de --reader          */
    char delimitador;                        /* ',' ';' '\t' ...           */
    char sep_decimal;                        /* '.' ou ',' nos numeros     */
    const char *const *colunas_obrigatorias; /* devem existir no cabecalho */
    int n_colunas_obrigatorias;
} ReaderPerfil;

extern const ReaderPerfil READER_PERFIL_NASA;
extern const ReaderPerfil READER_PERFIL_DETER;

#endif
