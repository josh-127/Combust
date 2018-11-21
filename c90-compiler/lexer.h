#ifndef PTC_CL_LEXER_H
#define PTC_CL_LEXER_H
#include "common.h"

#define TOKENFLAG_BOL 1

typedef enum {
    #define PpTok(name)       TK_PP_##name,
    #define PpKw(name)        TK_PP_##name,
    #define Tok(name)         TK_##name,
    #define Sym(name, symbol) TK_##name,
    #define Kw(name)          TK_KW_##name,

    #include "tokens.def"

    #undef Kw
    #undef Sym
    #undef Tok
    #undef PpKw
    #undef PpTok

    TOKEN_KIND_CARDINAL
} TOKEN_KIND;

typedef union {
    char   *IdentifierName;
    long    IntValue;
    float   FloatValue;
    double  DoubleValue;
    char   *StringValue;
    char    OffendingChar;
} TOKEN_VALUE;

typedef struct {
    int         Flags;
    SOURCE_LOC  Location;
    unsigned    Length;
    TOKEN_KIND  Kind;
    TOKEN_VALUE Value;
} TOKEN, *PTOKEN;

typedef struct tagLexer Lexer, *PLEXER;

PLEXER CreateLexer(PSOURCE_FILE input);
void   DeleteLexer(PLEXER l);
TOKEN  ReadTokenDirect(PLEXER l);
void   FreeToken(PTOKEN t);

#endif
