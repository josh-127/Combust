#include "logger.hh"
#include <stdarg.h>
#include <stdio.h>

int g_ErrorsLogged{ 0 };

#if defined(_WIN32)
#define RED       ""
#define GREEN     ""
#define YELLOW    ""
#define BLUE      ""
#define MAGENTA   ""
#define CYAN      ""
#define WHITE     ""
#define RED_B     ""
#define GREEN_B   ""
#define YELLOW_B  ""
#define BLUE_B    ""
#define MAGENTA_B ""
#define CYAN_B    ""
#define WHITE_B   ""
#else
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
#endif

static void LogMessage(const char *header, const char *format, va_list args) {
    fprintf(stderr, header, g_ProgramName);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
}

void Log(
    LOG_LEVEL   level,
    const char *format,
    ...
)
{
    va_list args;

    if (level >= LL_ERROR)
        ++g_ErrorsLogged;

    va_start(args, format);

    switch (level) {
        case LL_INFO:
            LogMessage(
                WHITE_B "%s: " WHITE,
                format,
                args
            );
            break;

        case LL_WARNING:
            LogMessage(
                WHITE_B "%s: " YELLOW_B "warning: " WHITE,
                format,
                args
            );
            break;

        case LL_ERROR:
            LogMessage(
                WHITE_B "%s: " RED_B "error: " WHITE,
                format,
                args
            );
            break;

        case LL_FATAL:
            LogMessage(
                WHITE_B "%s: " RED_B "fatal error: " WHITE,
                format,
                args
            );
            break;
    }

    va_end(args);
}

static void LogMessageAt(
    const char*  loc_msg,
    PCSOURCE_LOC loc,
    const char*  format,
    va_list      args
)
{
    std::string line{ loc->Source->GetLine(loc->Line) };

    fprintf(stderr, loc_msg,
            loc->Source->Name.c_str(), loc->Line + 1, loc->Column + 1);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    for (int i{ 0 }; line[i] != '\n'; ++i)
        fprintf(stderr, "%c", line[i]);
    fprintf(stderr, "\n");

    for (int i{ 0 }; i < loc->Column; ++i)
        fprintf(stderr, line[i] == '\t' ? "\t" : " ");
    fprintf(stderr, "^\n");
}

void LogAt(
    PCSOURCE_LOC loc,
    LOG_LEVEL    level,
    const char*  format,
    ...
)
{
    va_list args;

    if (level >= LL_ERROR)
        ++g_ErrorsLogged;

    va_start(args, format);
    
    switch (level) {
        case LL_INFO:
            LogMessageAt(
                WHITE_B "%s:%d:%d: " WHITE,
                loc,
                format,
                args
            );
            break;

        case LL_WARNING:
            LogMessageAt(
                WHITE_B "%s:%d:%d: " YELLOW_B "warning: " WHITE,
                loc,
                format,
                args
            );
            break;

        case LL_ERROR:
            LogMessageAt(
                WHITE_B "%s:%d:%d: " RED_B "error: " WHITE,
                loc,
                format,
                args
            );
            break;

        case LL_FATAL:
            LogMessageAt(
                WHITE_B "%s:%d:%d: " RED_B "fatal error: " WHITE,
                loc,
                format,
                args
            );
            break;
    }

    va_end(args);
}

static void LogMessageAtRange(
    const char*    loc_msg,
    PCSOURCE_RANGE range,
    const char*    format,
    va_list        args
)
    noexcept
{
    PCSOURCE_LOC loc{ &range->Location };
    Rc<const SourceFile> source{ loc->Source };
    std::string line{ source->GetLine(loc->Line) };

    fprintf(
        stderr,
        loc_msg,
        range->Location.Source->Name.c_str(),
        range->Location.Line + 1,
        range->Location.Column + 1
    );
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    for (char c : line) {
        if (c == '\t')
            fprintf(stderr, "    ");
        else
            fprintf(stderr, "%c", c);
    }
    fprintf(stderr, "\n");

    for (int i{ 0 }; i < range->Location.Column; ++i)
        fprintf(stderr, line[i] == '\t' ? "    " : " ");
    fprintf(stderr, "^");

    for (
        int i{ range->Location.Column };
        i < range->Location.Column + range->Length - 1;
        ++i
    )
    {
        fprintf(stderr, line[i] == '\t' ? "~~~~" : "~");
    }

    fprintf(stderr, "\n");
}

void LogAtRange(
    PCSOURCE_RANGE range,
    LOG_LEVEL      level,
    const char    *format,
    ...
)
{
    va_list args;
    
    if (level >= LL_ERROR)
        ++g_ErrorsLogged;

    va_start(args, format);

    switch (level) {
        case LL_INFO:
            LogMessageAtRange(
                WHITE_B "%s:%d:%d: " WHITE,
                range,
                format,
                args
            );
            break;

        case LL_WARNING:
            LogMessageAtRange(
                WHITE_B "%s:%d:%d: " YELLOW_B "warning: " WHITE,
                range,
                format,
                args
            );
            break;

        case LL_ERROR:
            LogMessageAtRange(
                WHITE_B "%s:%d:%d: " RED_B "error: " WHITE,
                range,
                format,
                args
            );
            break;

        case LL_FATAL:
            LogMessageAtRange(
                WHITE_B "%s:%d:%d: " RED_B "fatal error: " WHITE,
                range,
                format,
                args
            );
            break;
    }

    va_end(args);
}
