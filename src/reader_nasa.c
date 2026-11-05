#include "../include/reader_perfil.h"

/*
 * Perfil "nasa": asteroides (Parte 1).
 * CSV com virgula, ponto decimal e CRLF no fim da linha. Os campos Sentry
 * vem vazios quando o asteroide nao esta na lista, e os booleanos vem como
 * True/False (o item_numero transforma em 1/0).
 *
 * Todas as colunas abaixo precisam existir, mas a ordem no arquivo nao
 * importa (acessamos por nome) e coluna extra e tolerada. Assim, se abrirem
 * o arquivo errado o erro aparece logo, dizendo qual coluna falta, em vez
 * de uma consulta vazia sem explicacao.
 */
static const char *const COLUNAS_NASA[] = {
    "asteroid_designation", "asteroid_fullname",
    "close_approach_date", "year", "month",
    "distance_au", "velocity_km_s", "absolute_magnitude",
    "days_until_approach",
    "is_past_event", "is_future_event",
    "risk_score", "panic_level",
    "threat_category", "panic_verdict",
    "on_sentry_list", "sentry_impact_prob", "sentry_torino_scale",
    "sentry_palermo_scale", "sentry_diameter_km"
};

const ReaderPerfil READER_PERFIL_NASA = {
    "nasa",
    ',',
    '.',
    COLUNAS_NASA,
    (int)(sizeof COLUNAS_NASA / sizeof COLUNAS_NASA[0])
};
