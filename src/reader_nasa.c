#include "../include/reader_perfil.h"

/*
 * Perfil "nasa": registros de aproximacao de asteroides (Parte 1).
 *
 * Arquivo: CSV com delimitador ',', ponto decimal, terminador CRLF, campos
 * Sentry vazios quando o asteroide nao esta na lista. Campos booleanos usam
 * True/False, que item_numero converte em 1/0.
 *
 * Todas as colunas abaixo precisam existir; a ORDEM no arquivo nao importa
 * (os campos sao acessados pelo nome) e colunas extras sao toleradas. Assim,
 * abrir o arquivo errado falha cedo, apontando a coluna que falta, em vez de
 * devolver consultas vazias.
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
