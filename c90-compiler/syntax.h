#ifndef PTC_CL_SYNTAX_H
#define PTC_CL_SYNTAX_H

typedef enum tagSYNTAX_KIND {
#define SK(name) SK_##name,
#include "syntax-kinds.def"
#undef SK

    SYNTAX_KIND_CARDINAL
} SYNTAX_KIND;

typedef struct tagSYNTAX_NODE {
    SYNTAX_KIND  Kind;
    const char  *Lexeme;
    const char  *LeadingTrivia;
    const char  *TrailingTrivia;
} SYNTAX_NODE, *PSYNTAX_NODE;

typedef union tagSYNTAX_TOKEN_VALUE {
    const char *IdentifierName;
    long        IntValue;
    float       FloatValue;
    double      DoubleValue;
    const char *StringValue;
    char        OffendingChar;
} SYNTAX_TOKEN_VALUE, *PSYNTAX_TOKEN_VALUE;

typedef struct tagSYNTAX_TOKEN {
    SYNTAX_NODE        Base;
    SYNTAX_TOKEN_VALUE Value;
} SYNTAX_TOKEN, *PSYNTAX_TOKEN;

#endif
