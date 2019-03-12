#ifndef COMBUST_LOGGER_HH
#define COMBUST_LOGGER_HH
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
    PCSOURCE_LOC loc,
    LOG_LEVEL    level,
    const char*  format,
    ...
);

void LogAtRange(
    PCSOURCE_RANGE range,
    LOG_LEVEL      level,
    const char*    format,
    ...
);

#endif
