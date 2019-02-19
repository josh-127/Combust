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

struct SyntaxNode {
    SYNTAX_KIND   Kind;
    SOURCE_RANGE  LexemeRange;
};

struct SYNTAX_TOKEN_VALUE {
    std::string IdentifierName;
    long        IntValue;
    float       FloatValue;
    double      DoubleValue;
    std::string StringValue;
    char        OffendingChar;
};

struct SyntaxToken : public SyntaxNode {
    SYNTAX_TOKEN_VALUE Value;
    uint32_t           Flags;
};

#endif
