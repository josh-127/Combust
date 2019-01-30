#ifndef PTC_CL_SOURCE_H
#define PTC_CL_SOURCE_H
#include "common.hh"

struct SOURCE_FILE {
    char  *FileName;
    char  *Contents;
    char **Lines;
};

using PSOURCE_FILE = SOURCE_FILE*;

struct SOURCE_LOC {
    SOURCE_FILE *Source;
    int          Line;
    int          Column;
};

using PSOURCE_LOC = SOURCE_LOC*;

struct SOURCE_RANGE {
    SOURCE_LOC Location;
    int        Length;
};

using PSOURCE_RANGE = SOURCE_RANGE*;

int  OpenSourceFile(
    IN  const char   *fileName,
    OUT PSOURCE_FILE  sourceFile
);
void CloseSourceFile(
    THIS SOURCE_FILE *obj
);

#endif
