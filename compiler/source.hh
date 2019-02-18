#ifndef PTC_CL_SOURCE_H
#define PTC_CL_SOURCE_H
#include "common.hh"

struct SourceFile {
    SourceFile(IN const char* fileName);
    virtual ~SourceFile();

    char*  FileName;
    char*  Contents;
    char** Lines;
    bool   IsOpen;
};

struct SOURCE_LOC {
    SourceFile* Source;
    int         Line;
    int         Column;
};
using PSOURCE_LOC = SOURCE_LOC*;

struct SOURCE_RANGE {
    SOURCE_LOC Location;
    int        Length;
};
using PSOURCE_RANGE = SOURCE_RANGE*;

#endif
