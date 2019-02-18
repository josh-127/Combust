#ifndef PTC_CL_SOURCE_H
#define PTC_CL_SOURCE_H
#include "common.hh"
#include <string>
#include <vector>

struct SourceFile {
    SourceFile(const std::string& fileName);
    virtual ~SourceFile();

    const std::string& FileName;
    std::vector<char> Contents;
    char*  Data;
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
