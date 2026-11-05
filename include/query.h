#ifndef QUERY_H
#define QUERY_H

#include <stdbool.h>
#include "estruturas/item.h"

/*
 * TAD Query: le o arquivo de consulta (formato do PDF) e responde se um
 * ITEM passa na busca ou nao.
 *
 * Formato aceito (uma instrucao por linha, campos separados por '|'; linha
 * em branco e linha que comeca com '#' sao ignoradas):
 *
 *   FILTER|<campo>|==|<alvo>
 *   FILTER|<campo>|!=|<alvo>
 *   FILTER|<campo>|<=|<alvo_num>
 *   FILTER|<campo>|>=|<alvo_num>
 *   FILTER|<campo>|<|<alvo_num>
 *   FILTER|<campo>|>|<alvo_num>
 *   SIM|<campo>|<alvo>|<tolerancia>|<peso>
 *   ACCEPT|<limiar>
 *   ORDER|<campo ou similarity>|ASC ou DESC
 *   GROUP|<campo1>[|<campo2>]   (1 ou 2 campos)
 *   CROSS|<campo1>[|<campo2>]   (1 ou 2 campos)
 *
 * Como funciona:
 *   - Os FILTERs sao todos AND: se um falhar, o item esta fora.
 *   - Similaridade total = media ponderada dos SIM; sem SIM, vale 1.0.
 *   - Com SIM e ACCEPT, o total precisa ser >= limiar.
 *   - Quando o query_accepts aceita um item, ele guarda a similaridade dele
 *     pro ORDER|similarity (da pra ler com query_last_similarity).
 *
 * Memoria: quem chama query_load e dono do Query e libera com
 * query_destroy. As strings de query_order_field/group_fields/cross_fields
 * sao do Query e valem ate o destroy. A Estrutura copia so o nome do ORDER;
 * GROUP e CROSS ela so pega emprestado, entao o Query tem que viver mais
 * que ela.
 */

typedef struct Query Query;

// carrega o arquivo; NULL se der erro de I/O ou de sintaxe (o detalhe vai pro stderr)
Query *query_load(const char *caminho);

// libera; pode passar NULL
void query_destroy(Query *q);

/* Testa se o item passa nos FILTERs e, se tiver SIM, no ACCEPT.
 * Se aceitar, guarda a similaridade pro ORDER|similarity. */
bool query_accepts(const Query *q, const ITEM *it);

/* Similaridade total do ultimo item aceito pelo query_accepts. Vale 1.0 se
 * a query nao tem SIM ou se nenhum item foi aceito ainda. */
double query_last_similarity(const Query *q);

// ORDER
const char *query_order_field(const Query *q);
bool        query_order_desc(const Query *q);

// GROUP e CROSS
const char *const *query_group_fields(const Query *q);
int                query_group_nfields(const Query *q);
const char *const *query_cross_fields(const Query *q);
int                query_cross_nfields(const Query *q);

#endif