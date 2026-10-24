#include "../include/reader_perfil.h"

static const char *const COLUNAS_DETER[] = {
    "FID", "CLASSNAME", "QUADRANT", "PATH_ROW", "VIEW_DATE",
    "SENSOR", "SATELLITE",
    "AREAUCKM", "UC", "AREAMUNKM",
    "MUNICIPALI", "GEOCODIBGE", "UF"
};

const ReaderPerfil READER_PERFIL_DETER = {
    "deter",
    ',',
    '.',
    COLUNAS_DETER,
    (int)(sizeof COLUNAS_DETER / sizeof COLUNAS_DETER[0])
};