#ifndef PTC_CL_ERROR_H
#define PTC_CL_ERROR_H
#include "common.h"
#include "source.h"

extern char *g_ProgramName;

int g_ErrorsLogged;

typedef enum tagLOG_LEVEL {
    LL_INFO,
    LL_WARNING,
    LL_ERROR,
    LL_FATAL
} LOG_LEVEL;

void Log(
    LOG_LEVEL   level,
    const char *format,
    ...
);

void LogAt(
    PSOURCE_LOC  loc,
    LOG_LEVEL    level,
    const char  *format,
    ...
);

void LogAtRange(
    PSOURCE_RANGE  range,
    LOG_LEVEL    level,
    const char    *format,
    ...
);

#endif
