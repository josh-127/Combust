#ifndef COMBUST_SOURCE_HH
#define COMBUST_SOURCE_HH
#include "common.hh"
#include <string>
#include <vector>

class SourceFile {
public:
    explicit SourceFile(const std::string& name, const std::vector<char>& contents);
    virtual ~SourceFile();

    std::string GetLine(int line) const;

    const std::string Name;
    const std::vector<char> Contents;

private:
    std::vector<int> lines;
};

Rc<SourceFile> CreateSourceFile(
    const std::string& name,
    const std::string& contents
);
Rc<SourceFile> OpenSourceFile(const std::string& path);

struct SourceLoc {
    Rc<const SourceFile> Source{ };
    int                  Line{ 0 };
    int                  Column{ 0 };
};
using PSOURCE_LOC = SourceLoc*;
using PCSOURCE_LOC = const SourceLoc*;

struct SourceRange {
    SourceLoc Location{ };
    int       Length{ };
};
using PSOURCE_RANGE = SourceRange*;
using PCSOURCE_RANGE = const SourceRange*;

#endif
