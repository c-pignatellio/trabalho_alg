#ifndef READER_PERFIL_H
#define READER_PERFIL_H

/*
 * Perfil de um dataset: tudo que o Reader precisa saber sobre o formato do
 * arquivo. E de uso interno dos leitores (reader.c e reader_<dataset>.c);
 * o resto do programa so enxerga o reader.h.
 *
 * Pra adicionar um dataset (ex.: "compras"):
 *   1. criar src/reader_compras.c definindo READER_PERFIL_COMPRAS;
 *   2. declarar o extern abaixo;
 *   3. colocar o perfil na tabela PERFIS do reader.c.
 */

typedef struct {
    const char *nome;  // valor do --reader
    char delimitador;  // ',' ';' '\t' ...
    char sep_decimal;  // '.' ou ',' nos numeros
    const char *const *colunas_obrigatorias;  // tem que existir no cabecalho
    int n_colunas_obrigatorias;
} ReaderPerfil;

extern const ReaderPerfil READER_PERFIL_NASA;
extern const ReaderPerfil READER_PERFIL_DETER;

#endif
