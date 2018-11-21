#ifndef PTC_CL_COMMON_H
#define PTC_CL_COMMON_H

extern char *g_ProgramName;

/* source.c */
typedef struct {
    char  *FileName;
    char  *Contents;
    char **Lines;
} SOURCE_FILE, *PSOURCE_FILE;

typedef struct {
    SOURCE_FILE *Source;
    int          Line;
    int          Column;
} SOURCE_LOC, *PSOURCE_LOC;

typedef struct {
    SOURCE_LOC Base;
    int        Length;
} SOURCE_RANGE, *PSOURCE_RANGE;

int  OpenSourceFile(const char *fileName, PSOURCE_FILE sourceFile);
void CloseSourceFile(SOURCE_FILE *obj);

/* error.c */
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
