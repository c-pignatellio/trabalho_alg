#ifndef ITEM_H
#define ITEM_H

#include <stdbool.h>
#include <stdio.h>

/* TAD ITEM: um registro de qualquer dataset tabular delimitado.

 Posse de memoria: ITEM usa contagem de referencias. item_criar devolve
 ref = 1; cada estrutura que guardar o mesmo ITEM em outra colecao chama item_referenciar. 
 item_apagar apenas decrementa e libera quando chega a zero, entao o mesmo registro pode estar
 na base e no resultado sem copia e sem double free.
 */

//Descreve as colunas do arquivo. Compartilhado por todos os itens.
typedef struct esquema_ ESQUEMA;

/* linha_cabecalho: primeira linha do arquivo (com ou sem \r\n).
 * delimitador:     ',' ';' '\t' ...
 * sep_decimal:     '.' ou ',' (dados.gov.br costuma usar ','). */
ESQUEMA *esquema_criar(const char *linha_cabecalho, char delimitador, char sep_decimal);
int esquema_num_campos(const ESQUEMA *e);
const char *esquema_nome_campo(const ESQUEMA *e, int idx);
int esquema_indice(const ESQUEMA *e, const char *nome);   /* -1 se nao existe */
void esquema_escrever_cabecalho(const ESQUEMA *e, FILE *saida);
void esquema_apagar(ESQUEMA **e);


typedef struct item_ ITEM;

/* Cria um item a partir de uma linha de dados. O esquema NAO e copiado e
 * deve sobreviver a todos os itens. Campos ausentes/vazios sao aceitos. */
ITEM *item_criar(const ESQUEMA *e, const char *linha);

/* Registra mais um dono para o item e o devolve (permite encadear). */
ITEM *item_referenciar(ITEM *item);

/* Libera uma referencia; destroi o item na ultima. Zera *item. */
void item_apagar(ITEM **item);

const ESQUEMA *item_esquema(const ITEM *item);

/* Texto do campo ja sem aspas externas. "" se ausente ou idx invalido. */
const char *item_texto(const ITEM *item, int idx);

/* true se o campo esta vazio/ausente. */
bool item_ausente(const ITEM *item, int idx);

/* Valor numerico do campo. Aceita numeros e booleanos (true/false -> 1/0,
 * sem diferenciar maiusculas). Devolve false se ausente ou nao numerico. */
bool item_numero(const ITEM *item, int idx, double *valor);

/* Escreve a linha original, byte a byte, no mesmo formato da entrada.*/
void item_escrever(const ITEM *item, FILE *saida);

#endif 