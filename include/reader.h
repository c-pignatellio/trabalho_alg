#ifndef READER_H
#define READER_H

#include <stdbool.h>
#include "item.h"

/*
 * TAD Reader: transforma um arquivo de dados em uma sequencia de ITENS.
 *
 * E o UNICO modulo que depende do dataset. As estruturas (pilha, fila,
 * listas...) so enxergam ITEM, entao trocar de dataset e trocar o nome em
 * --reader (ex.: "nasa", "compras", "contratos"), sem tocar nas estruturas.
 *
 * Divisao de trabalho:
 *   reader.c        mecanica generica: abrir arquivo, ler linhas de qualquer
 *                   tamanho, montar o ESQUEMA, criar ITENS, contar erros.
 *   reader_nasa.c   perfil do dataset (delimitador, decimal, colunas que o
 *                   arquivo precisa ter). Um dataset novo = um perfil novo.
 *
 * Uso tipico:
 *   Reader *r = reader_create("nasa");
 *   if (r == NULL || !reader_open(r, caminho)) { ... reader_erro(r) ... }
 *   ITEM *it;
 *   while ((it = reader_next(r)) != NULL) { ... }
 *   ...
 *   reader_destroy(r);     // DEPOIS de destruir as estruturas (ver abaixo)
 *
 * POSSE DE MEMORIA
 *   - reader_next devolve ITEM com ref = 1; quem recebe e responsavel por
 *     item_apagar (normalmente a estrutura --base, via owns_items).
 *   - O Reader e dono do ESQUEMA. Com o item.h atual o esquema NAO e
 *     contado por referencia, entao reader_destroy so pode ser chamado
 *     depois que todos os itens ja foram apagados.
 */

typedef struct Reader Reader;

/* Cria o leitor do dataset "nome" (ex.: "nasa"). NULL se nao existir. */
Reader *reader_create(const char *nome);

/* Nomes dos leitores disponiveis, para mensagens de uso. NULL ao acabar:
 *   for (int i = 0; reader_nome(i); i++) puts(reader_nome(i));          */
const char *reader_nome(int indice);

/* Abre o arquivo, le o cabecalho e valida as colunas exigidas pelo perfil.
 * Devolve false em falha (detalhe em reader_erro). So pode ser chamado uma
 * vez por Reader. */
bool reader_open(Reader *r, const char *caminho);

/* Esquema do arquivo (colunas). Valido ate reader_destroy. NULL se nao
 * aberto. Serve para o writer reproduzir o cabecalho original:
 *   esquema_escrever_cabecalho(reader_esquema(r), arquivo);              */
const ESQUEMA *reader_esquema(const Reader *r);

/* Proximo registro valido, ou NULL no fim do arquivo. Linhas em branco sao
 * puladas; linhas malformadas (aspas invalidas, numero de campos diferente
 * do cabecalho) sao puladas e contadas, sem interromper a leitura.
 * Para distinguir fim de arquivo de falha de leitura, confira reader_erro()
 * apos receber NULL. */
ITEM *reader_next(Reader *r);

/* Quantas linhas de dados foram puladas por estarem malformadas. */
long reader_linhas_ignoradas(const Reader *r);

/* Numero (base 1, contando o cabecalho como linha 1) da primeira linha
 * malformada, ou 0 se nenhuma. */
long reader_primeira_linha_invalida(const Reader *r);

/* Ultima mensagem de erro, ou "" se nao houve. */
const char *reader_erro(const Reader *r);

/* Fecha o arquivo. Itens ja lidos continuam validos. */
void reader_close(Reader *r);

/* Fecha (se preciso) e libera o leitor e o esquema. */
void reader_destroy(Reader *r);

#endif
