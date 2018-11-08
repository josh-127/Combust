#ifndef MYCC_COMMON_H
#define MYCC_COMMON_H

extern char *g_ProgramName;

/* source.c */
typedef struct {
    char  *FileName;
    char  *Contents;
    char **Lines;
} SOURCE_FILE;

typedef struct {
    SOURCE_FILE *Source;
    unsigned     Line;
    unsigned     Column;
} SOURCE_LOC;

SOURCE_FILE OpenSourceFile(const char *fname);
void        CloseSourceFile(SOURCE_FILE *sf);

/* error.c */
unsigned g_ErrorsLogged;

void LogError   (const char *format, ...);
void LogWarning (const char *format, ...);
void LogFatal   (const char *format, ...);
void LogErrorC  (SOURCE_LOC *loc, const char *format, ...);
void LogWarningC(SOURCE_LOC *loc, const char *format, ...);
void LogFatalC  (SOURCE_LOC *loc, const char *format, ...);


#endif
