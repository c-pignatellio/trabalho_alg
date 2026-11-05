#ifndef READER_H
#define READER_H

#include <stdbool.h>
#include "item.h"

/*
 * TAD Reader: transforma um arquivo de dados numa sequencia de ITENS.
 *
 * E o unico modulo que depende do dataset. As estruturas (pilha, fila,
 * listas...) so enxergam ITEM, entao trocar de dataset e so trocar o nome
 * no --reader (ex.: "nasa", "compras", "contratos"), sem mexer nelas.
 *
 * Divisao do trabalho:
 *   reader.c       a parte generica: abrir o arquivo, ler linhas de qualquer
 *                  tamanho, montar o ESQUEMA, criar os ITENS, contar erros.
 *   reader_nasa.c  o perfil do dataset (delimitador, decimal, colunas que o
 *                  arquivo precisa ter). Dataset novo = perfil novo.
 *
 * Uso tipico:
 *   Reader *r = reader_create("nasa");
 *   if (r == NULL || !reader_open(r, caminho)) { ... reader_erro(r) ... }
 *   ITEM *it;
 *   while ((it = reader_next(r)) != NULL) { ... }
 *   reader_destroy(r);   // so DEPOIS de destruir as estruturas (veja abaixo)
 *
 * Quem e dono de que:
 *   - reader_next devolve ITEM com ref = 1; quem recebe tem que chamar
 *     item_apagar (normalmente a estrutura --base, via owns_items).
 *   - O Reader e dono do ESQUEMA. Como o esquema nao tem contagem de
 *     referencias, o reader_destroy so pode rodar depois que todos os itens
 *     ja foram apagados.
 */

typedef struct Reader Reader;

// cria o leitor do dataset "nome" (ex.: "nasa"); NULL se nao existir
Reader *reader_create(const char *nome);

/* Nomes dos leitores disponiveis, pra mensagem de uso. Devolve NULL no fim:
 *   for (int i = 0; reader_nome(i); i++) puts(reader_nome(i)); */
const char *reader_nome(int indice);

/* Abre o arquivo, le o cabecalho e confere as colunas que o perfil exige.
 * Devolve false se falhar (o motivo fica em reader_erro). So pode ser
 * chamado uma vez por Reader. */
bool reader_open(Reader *r, const char *caminho);

/* Esquema do arquivo (colunas), valido ate o reader_destroy. NULL se ainda
 * nao abriu. O writer usa isso pra repetir o cabecalho original:
 *   esquema_escrever_cabecalho(reader_esquema(r), arquivo); */
const ESQUEMA *reader_esquema(const Reader *r);

/* Proximo registro valido, ou NULL no fim do arquivo. Linha em branco e
 * pulada; linha malformada (aspas erradas, numero de campos diferente do
 * cabecalho) tambem e pulada e contada, sem parar a leitura. Pra saber se
 * o NULL foi fim de arquivo ou falha de leitura, olhe o reader_erro(). */
ITEM *reader_next(Reader *r);

// quantas linhas de dados foram puladas por estarem malformadas
long reader_linhas_ignoradas(const Reader *r);

/* Numero da primeira linha malformada (base 1, o cabecalho e a linha 1),
 * ou 0 se nao teve nenhuma. */
long reader_primeira_linha_invalida(const Reader *r);

// ultima mensagem de erro, ou "" se nao teve
const char *reader_erro(const Reader *r);

// fecha o arquivo; os itens ja lidos continuam valendo
void reader_close(Reader *r);

// fecha (se ainda estiver aberto) e libera o leitor e o esquema
void reader_destroy(Reader *r);

#endif
