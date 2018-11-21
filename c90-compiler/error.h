#ifndef PTC_CL_ERROR_H
#define PTC_CL_ERROR_H
#include "source.h"

extern char *g_ProgramName;

int g_ErrorsLogged;

void LogError         (const char *format, ...);
void LogWarning       (const char *format, ...);
void LogFatal         (const char *format, ...);
void LogErrorAt       (PSOURCE_LOC loc, const char *format, ...);
void LogWarningAt     (PSOURCE_LOC loc, const char *format, ...);
void LogFatalAt       (PSOURCE_LOC loc, const char *format, ...);
void LogErrorAtRange  (PSOURCE_RANGE range, const char *format, ...);
void LogWarningAtRange(PSOURCE_RANGE range, const char *format, ...);
void LogFatalAtRange  (PSOURCE_RANGE range, const char *format, ...);

#endif
