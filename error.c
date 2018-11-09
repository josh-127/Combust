#include "common.h"
#include <stdarg.h>
#include <stdio.h>

unsigned g_ErrorsLogged = 0;

#define RED       "\x1b[31m"
#define GREEN     "\x1b[32m"
#define YELLOW    "\x1b[33m"
#define BLUE      "\x1b[34m"
#define MAGENTA   "\x1b[35m"
#define CYAN      "\x1b[36m"
#define WHITE     "\x1b[0m"
#define RED_B     "\x1b[1m\x1b[31m"
#define GREEN_B   "\x1b[1m\x1b[32m"
#define YELLOW_B  "\x1b[1m\x1b[33m"
#define BLUE_B    "\x1b[1m\x1b[34m"
#define MAGENTA_B "\x1b[1m\x1b[35m"
#define CYAN_B    "\x1b[1m\x1b[36m"
#define WHITE_B   "\x1b[1m\x1b[0m"

static void LogMessage(const char *header, const char *format, va_list args)
{
    fprintf(stderr, header, g_ProgramName);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
}

void LogError(const char *format, ...)
{
    ++g_ErrorsLogged;
    va_list args;
    va_start(args, format);
    LogMessage(WHITE_B "%s: " RED_B "error: " WHITE, format, args);
    va_end(args);
}

void LogWarning(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    LogMessage(WHITE_B "%s: " YELLOW_B "warning: " WHITE, format, args);
    va_end(args);
}

void LogFatal(const char *format, ...)
{
    ++g_ErrorsLogged;
    va_list args;
    va_start(args, format);
    LogMessage(WHITE_B "%s: " RED_B "fatal error: " WHITE, format, args);
    va_end(args);
}

static void LogMessageC(
    const char *loc_msg,
    PSOURCE_LOC loc,
    const char *format,
    va_list     args
)
{
    unsigned i;
    char *line = loc->Source->Lines[loc->Line];

    fprintf(stderr, loc_msg,
            loc->Source->FileName, loc->Line + 1, loc->Column + 1);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    for (i = 0; line[i] != '\n'; ++i)
        fprintf(stderr, "%c", line[i]);
    fprintf(stderr, "\n");

    for (i = 0; i < loc->Column; ++i)
        fprintf(stderr, line[i] == '\t' ? "\t" : " ");
    fprintf(stderr, "^\n");
}

void LogErrorC(PSOURCE_LOC loc, const char *format, ...)
{
    ++g_ErrorsLogged;
    va_list args;
    va_start(args, format);
    LogMessageC(WHITE_B "%s:%d:%d: " RED_B "error: " WHITE, loc, format, args);
    va_end(args);
}

void LogWarningC(PSOURCE_LOC loc, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    LogMessageC(WHITE_B "%s:%d:%d: " YELLOW_B "warning: " WHITE, loc, format, args);
    va_end(args);
}

void LogFatalC(PSOURCE_LOC loc, const char *format, ...)
{
    ++g_ErrorsLogged;
    va_list args;
    va_start(args, format);
    LogMessageC(WHITE_B "%s:%d:%d: " RED_B "fatal error: " WHITE, loc, format, args);
    va_end(args);
}
