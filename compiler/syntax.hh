#ifndef PTC_CL_SYNTAX_H
#define PTC_CL_SYNTAX_H
#include "common.hh"
#include "source.hh"
#include <stdint.h>
#include <string>

#define ST_BEGINNING_OF_LINE 1

enum SYNTAX_KIND {
    SK_UNINITIALIZED,

#define Token(name) SK_##name,
#define Node(name) SK_##name,
#include "syntax-kinds.def"
#undef Token
#undef Node

    SYNTAX_KIND_CARDINAL
};

class SyntaxNode {
public:
    SyntaxNode() {}
    SyntaxNode(const SyntaxNode& obj) :
        kind{ obj.kind },
        lexemeRange{ obj.lexemeRange } {}
    virtual ~SyntaxNode() {}
    SYNTAX_KIND GetKind() const { return kind; }
    void SetKind(const SYNTAX_KIND to) { kind = to; }
    SOURCE_RANGE GetLexemeRange() const { return lexemeRange; }
    void SetLexemeRange(const SOURCE_RANGE& to) { lexemeRange = to; }
private:
    SYNTAX_KIND kind{ SK_UNINITIALIZED };
    SOURCE_RANGE lexemeRange{ };
};

struct SYNTAX_TOKEN_VALUE {
};

class SyntaxToken : public SyntaxNode {
public:
    SyntaxToken() {}
    SyntaxToken(const SyntaxToken& obj) :
        SyntaxNode{ obj },
        identifierName{ obj.identifierName },
        intValue{ obj.intValue },
        floatValue{ obj.floatValue },
        doubleValue{ obj.doubleValue },
        stringValue{ obj.stringValue },
        offendingChar{ obj.offendingChar },
        flags{ obj.flags } {}
    virtual ~SyntaxToken() {}
    std::string GetName() const { return identifierName; }
    void SetName(const std::string& to) { identifierName = to; }
    long GetIntValue() const { return intValue; }
    void SetIntValue(const long to) { intValue = to; }
    float GetFloatValue() const { return floatValue; }
    void SetFloatValue(const float to) { floatValue = to; }
    double GetDoubleValue() const { return doubleValue; }
    void SetDoubleValue(const double to) { doubleValue = to; }
    std::string GetStringValue() const { return stringValue; }
    void SetStringValue(const std::string& to) { stringValue = to; }
    char GetOffendingChar() const { return offendingChar; }
    void SetOffendingChar(const char to) { offendingChar = to; }
    uint32_t GetFlags() const { return flags; }
    void SetFlags(const uint32_t to) { flags = to; }
private:
    std::string identifierName{ };
    long        intValue{ 0L };
    float       floatValue{ 0.0F };
    double      doubleValue{ 0.0 };
    std::string stringValue{ };
    char        offendingChar{ 0 };
    uint32_t    flags{ 0 };
};

#endif
