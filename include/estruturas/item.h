#ifndef ITEM_H
#define ITEM_H

#include <stdbool.h>
#include <stdio.h>

//Descreve as colunas do arquivo. Compartilhado por todos os itens
typedef struct esquema_ ESQUEMA;

ESQUEMA *esquema_criar(const char *linha_cabecalho, char delimitador, char sep_decimal);
int esquema_num_campos(const ESQUEMA *e);
const char *esquema_nome_campo(const ESQUEMA *e, int idx);
int esquema_indice(const ESQUEMA *e, const char *nome); // -1 se não existe
void esquema_escrever_cabecalho(const ESQUEMA *e, FILE *saida);
void esquema_apagar(ESQUEMA **e);


typedef struct item_ ITEM;

// Cria um item a partir de uma linha de dados
ITEM *item_criar(const ESQUEMA *e, const char *linha);

// Registra mais um dono para o item e o devolve (permite encadear)
ITEM *item_referenciar(ITEM *item);

// Libera uma referência
void item_apagar(ITEM **item);

const ESQUEMA *item_esquema(const ITEM *item);

// Texto do campo ja sem aspas externas. "" se ausente ou idx invalido
const char *item_texto(const ITEM *item, int idx);

// true se o campo esta vazio/ausente
bool item_ausente(const ITEM *item, int idx);

// Valor numerico do campo. Aceita numeros e booleanos (true/false -> 1/0
bool item_numero(const ITEM *item, int idx, double *valor);

// Escreve a linha original, byte a byte, no mesmo formato da entrada
void item_escrever(const ITEM *item, FILE *saida);

#endif 