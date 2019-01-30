#ifndef PTC_CL_SYNTAX_H
#define PTC_CL_SYNTAX_H
#include "common.hh"
#include "source.hh"
#include <stdint.h>

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

struct SYNTAX_NODE {
    SYNTAX_KIND   Kind;
    SOURCE_RANGE  LexemeRange;
};

using PSYNTAX_NODE = SYNTAX_NODE*;

union SYNTAX_TOKEN_VALUE {
    char   *IdentifierName;
    long    IntValue;
    float   FloatValue;
    double  DoubleValue;
    char   *StringValue;
    char    OffendingChar;
};

using PSYNTAX_TOKEN_VALUE = SYNTAX_TOKEN_VALUE*;

struct SYNTAX_TOKEN {
    SYNTAX_NODE        Base;
    SYNTAX_TOKEN_VALUE Value;
    uint32_t           Flags;
};

using PSYNTAX_TOKEN = SYNTAX_TOKEN*;

void DeleteSyntaxNode(
    THIS PSYNTAX_NODE obj
);

#endif
