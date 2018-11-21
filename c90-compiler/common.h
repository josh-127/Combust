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

SOURCE_FILE OpenSourceFile(const char *fileName);
void        CloseSourceFile(SOURCE_FILE *obj);

/* error.c */
int g_ErrorsLogged;

void LogError   (const char *format, ...);
void LogWarning (const char *format, ...);
void LogFatal   (const char *format, ...);
void LogErrorC  (PSOURCE_LOC loc, const char *format, ...);
void LogWarningC(PSOURCE_LOC loc, const char *format, ...);
void LogFatalC  (PSOURCE_LOC loc, const char *format, ...);


#endif
