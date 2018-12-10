#ifndef PTC_CL_SYNTAX_H
#define PTC_CL_SYNTAX_H
#include "common.h"
#include "source.h"
#include <stdint.h>

#define ST_BEGINNING_OF_LINE 1

typedef enum tagSYNTAX_KIND {
    SK_UNINITIALIZED,

#define Token(name) SK_##name,
#define Node(name) SK_##name,
#include "syntax-kinds.def"
#undef Token
#undef Node

    SYNTAX_KIND_CARDINAL
} SYNTAX_KIND;

typedef struct tagSYNTAX_NODE {
    SYNTAX_KIND   Kind;
    SOURCE_RANGE  LexemeRange;
} SYNTAX_NODE, *PSYNTAX_NODE;

typedef union tagSYNTAX_TOKEN_VALUE {
    char   *IdentifierName;
    long    IntValue;
    float   FloatValue;
    double  DoubleValue;
    char   *StringValue;
    char    OffendingChar;
} SYNTAX_TOKEN_VALUE, *PSYNTAX_TOKEN_VALUE;

typedef struct tagSYNTAX_TOKEN {
    SYNTAX_NODE        Base;
    SYNTAX_TOKEN_VALUE Value;
    uint32_t           Flags;
} SYNTAX_TOKEN, *PSYNTAX_TOKEN;

void DeleteSyntaxNode(
    THIS PSYNTAX_NODE obj
);

#endif
