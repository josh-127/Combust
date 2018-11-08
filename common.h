#ifndef COMMON_H
#define COMMON_H

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

extern SOURCE_FILE OpenSourceFile(const char *fname);
extern void        CloseSourceFile(SOURCE_FILE *sf);

/* error.c */
extern unsigned g_ErrorsLogged;

extern void LogError   (const char *format, ...);
extern void LogWarning (const char *format, ...);
extern void LogFatal   (const char *format, ...);
extern void LogErrorC  (SOURCE_LOC *loc, const char *format, ...);
extern void LogWarningC(SOURCE_LOC *loc, const char *format, ...);
extern void LogFatalC  (SOURCE_LOC *loc, const char *format, ...);


#endif
