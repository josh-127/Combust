#ifndef PTC_CL_SYNTAX_H
#define PTC_CL_SYNTAX_H
#include <stdint.h>

#define ST_BEGINNING_OF_LINE 1

typedef enum tagSYNTAX_KIND {
#define SK(name) SK_##name,
#include "syntax-kinds.def"
#undef SK

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

#endif
