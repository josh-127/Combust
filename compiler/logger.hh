#ifndef PTC_CL_LOGGER_H
#define PTC_CL_LOGGER_H
#include "common.hh"
#include "source.hh"

extern char *g_ProgramName;

extern int g_ErrorsLogged;

enum LOG_LEVEL {
    LL_INFO,
    LL_WARNING,
    LL_ERROR,
    LL_FATAL
};

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
