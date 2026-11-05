#ifndef ITEM_H
#define ITEM_H

#include <stdbool.h>
#include <stdio.h>

/* TAD ITEM: um registro de qualquer dataset tabular delimitado.
 *
 * Quem e dono do ITEM: ele tem contagem de referencias. item_criar devolve
 * ref = 1, item_referenciar soma uma e item_apagar tira uma, liberando
 * quando chega a zero. No programa de hoje so a estrutura --base e dona dos
 * itens; a --result guarda ponteiros emprestados (owns_items = false), sem
 * copia e sem double free.
 */

// descreve as colunas do arquivo; todos os itens compartilham o mesmo
typedef struct esquema_ ESQUEMA;

/* linha_cabecalho: primeira linha do arquivo (com ou sem \r\n).
 * delimitador: ',' ';' '\t' ...
 * sep_decimal: '.' ou ',' (o dados.gov.br costuma usar ','). */
ESQUEMA *esquema_criar(const char *linha_cabecalho, char delimitador, char sep_decimal);
int esquema_num_campos(const ESQUEMA *e);
const char *esquema_nome_campo(const ESQUEMA *e, int idx);
int esquema_indice(const ESQUEMA *e, const char *nome);  // -1 se nao existe
void esquema_escrever_cabecalho(const ESQUEMA *e, FILE *saida);
void esquema_apagar(ESQUEMA **e);


typedef struct item_ ITEM;

/* Cria um item a partir de uma linha de dados. O esquema nao e copiado e
 * precisa viver mais que todos os itens. Campo vazio/ausente e aceito. */
ITEM *item_criar(const ESQUEMA *e, const char *linha);

// soma mais um dono pro item e devolve ele mesmo (da pra encadear)
ITEM *item_referenciar(ITEM *item);

// tira uma referencia e destroi o item quando for a ultima; zera *item
void item_apagar(ITEM **item);

const ESQUEMA *item_esquema(const ITEM *item);

// texto do campo ja sem as aspas de fora; "" se ausente ou idx invalido
const char *item_texto(const ITEM *item, int idx);

// true se o campo esta vazio ou ausente
bool item_ausente(const ITEM *item, int idx);

/* Valor numerico do campo. Aceita numero e booleano (true/false vira 1/0,
 * sem ligar pra maiuscula). Devolve false se ausente ou nao numerico. */
bool item_numero(const ITEM *item, int idx, double *valor);

// escreve a linha original, byte a byte, no mesmo formato da entrada
void item_escrever(const ITEM *item, FILE *saida);

#endif 