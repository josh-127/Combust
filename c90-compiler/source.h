#ifndef PTC_CL_SOURCE_H
#define PTC_CL_SOURCE_H

typedef struct tagSOURCE_FILE {
    char  *FileName;
    char  *Contents;
    char **Lines;
} SOURCE_FILE, *PSOURCE_FILE;

typedef struct tagSOURCE_LOC {
    SOURCE_FILE *Source;
    int          Line;
    int          Column;
} SOURCE_LOC, *PSOURCE_LOC;

typedef struct tagSOURCE_RANGE {
    SOURCE_LOC Base;
    int        Length;
} SOURCE_RANGE, *PSOURCE_RANGE;

int  OpenSourceFile(const char *fileName, PSOURCE_FILE sourceFile);
void CloseSourceFile(SOURCE_FILE *obj);

#endif
