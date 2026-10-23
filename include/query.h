#ifndef QUERY_H
#define QUERY_H

#include <stdbool.h>
#include "estruturas/item.h"

/*
 * TAD Query: le um arquivo de consulta (formato do PDF) e responde
 * "este ITEM satisfaz a busca?".
 *
 * Gramatica aceita (uma instrucao por linha; campos separados por '|';
 * linhas em branco e linhas iniciadas por '#' sao ignoradas):
 *
 *   FILTER|<campo>|==|<alvo>
 *   FILTER|<campo>|!=|<alvo>
 *   FILTER|<campo>|<=|<alvo_num>
 *   FILTER|<campo>|>=|<alvo_num>
 *   FILTER|<campo>|< |<alvo_num>
 *   FILTER|<campo>|> |<alvo_num>
 *
 *   SIM|<campo>|<alvo>|<tolerancia>|<peso>
 *
 *   ACCEPT|<limiar>
 *
 *   ORDER|<campo|similarity>|ASC|DESC
 *
 *   GROUP|<campo1>[|<campo2>]      (1 ou 2 campos)
 *   CROSS|<campo1>[|<campo2>]      (1 ou 2 campos)
 *
 * Semantica:
 *   - Todos os FILTERs sao conjuntivos (AND). Qualquer falha rejeita.
 *   - Similaridade total = media ponderada dos SIM; sem SIM, vale 1.0.
 *   - Se houver SIM e ACCEPT, exige-se total >= limiar.
 *   - query_accepts, ao aceitar, guarda a similaridade do item para
 *     ORDER|similarity (consultada via query_last_similarity).
 *
 * POSSE: quem chama e dono do Query devolvido por query_load e deve
 * libera-lo com query_destroy. As strings devolvidas por
 * query_order_field/query_group_fields/query_cross_fields pertencem ao
 * Query e valem ate query_destroy (a Estrutura copia os nomes que usa).
 */

typedef struct Query Query;

/* Carrega o arquivo. NULL em erro de I/O ou sintaxe (detalhe em stderr). */
Query *query_load(const char *caminho);

/* Libera. Seguro com NULL. */
void query_destroy(Query *q);

/* Testa se o item satisfaz FILTERs e (se houver SIM) ACCEPT.
 * Se aceitar, memoriza a similaridade para ORDER|similarity. */
bool query_accepts(const Query *q, const ITEM *it);

/* Similaridade total do ultimo item aceito por query_accepts. 1.0 se a
 * query nao tem SIM, ou se nenhum item foi aceito ainda. */
double query_last_similarity(const Query *q);

/* ORDER */
const char *query_order_field(const Query *q);
bool        query_order_desc(const Query *q);

/* GROUP / CROSS */
const char *const *query_group_fields(const Query *q);
int                query_group_nfields(const Query *q);
const char *const *query_cross_fields(const Query *q);
int                query_cross_nfields(const Query *q);

#endif