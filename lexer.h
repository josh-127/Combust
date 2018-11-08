#ifndef MYCC_LEXER_H
#define MYCC_LEXER_H
#include "common.h"

#define TOKENFLAG_BOL 1

typedef enum {
    TK_PP_HASH,
    TK_PP_IF,       TK_PP_IFDEF,    TK_PP_IFNDEF,
    TK_PP_ELIF,     TK_PP_ENDIF,
    TK_PP_INCLUDE,
    TK_PP_DEFINE,   TK_PP_UNDEF,
    TK_PP_LINE,     TK_PP_ERROR,    TK_PP_WARNING,

    TK_UNKNOWN,     TK_EOF,         TK_COMMENT,     TK_IDENTIFIER,
    TK_INT_CONSTANT,TK_FLOAT_CONSTANT,              TK_DOUBLE_CONSTANT,
    TK_STR_CONSTANT,TK_ANGLED_STR_CONSTANT,

    TK_LPAREN,      TK_RPAREN,
    TK_LBRACKET,    TK_RBRACKET,
    TK_LBRACE,      TK_RBRACE,
    TK_SEMICOLON,   TK_DOT,             TK_COMMA,
    TK_TILDE,       TK_QUESTION,        TK_COLON,
    TK_PLUS,        TK_PLUS_EQUALS,     TK_PLUS_PLUS,
    TK_MINUS,       TK_MINUS_EQUALS,    TK_MINUS_MINUS,     TK_MINUS_GT,
    TK_ASTERISK,    TK_ASTERISK_EQUALS,
    TK_SLASH,       TK_SLASH_EQUALS,
    TK_PERCENT,     TK_PERCENT_EQUALS,
    TK_LT,          TK_LT_EQUALS,   TK_LT_LT,       TK_LT_LT_EQUALS,
    TK_GT,          TK_GT_EQUALS,   TK_GT_GT,       TK_GT_GT_EQUALS,
    TK_EQUALS,      TK_EQUALS_EQUALS,
    TK_EXCLAMATION, TK_EXCLAMATION_EQUALS,
    TK_AMPERSAND,   TK_AMPERSAND_EQUALS,            TK_AMPERSAND_AMPERSAND,
    TK_CARET,       TK_CARET_EQUALS,
    TK_PIPE,        TK_PIPE_EQUALS,                 TK_PIPE_PIPE,

    TK_KW_CONST,    TK_KW_EXTERN,   TK_KW_STATIC,   TK_KW_AUTO,
    TK_KW_VOLATILE, TK_KW_UNSIGNED, TK_KW_SIGNED,   TK_KW_VOID,
    TK_KW_CHAR,     TK_KW_SHORT,    TK_KW_INT,      TK_KW_LONG,
    TK_KW_FLOAT,    TK_KW_DOUBLE,   TK_KW_ENUM,     TK_KW_STRUCT,
    TK_KW_UNION,    TK_KW_TYPEDEF,  TK_KW_SIZEOF,
    TK_KW_GOTO,     TK_KW_IF,       TK_KW_ELSE,     TK_KW_SWITCH,
    TK_KW_CASE,     TK_KW_DEFAULT,  TK_KW_DO,       TK_KW_WHILE,
    TK_KW_FOR,      TK_KW_BREAK,    TK_KW_CONTINUE, TK_KW_RETURN
} TOKEN_KIND;

typedef struct {
    int            Flags;
    SOURCE_LOC     Location;
    unsigned       Length;
    TOKEN_KIND     Kind;
    union {
        char      *IdentifierName;
        long long  IntValue;
        float      FloatValue;
        double     DoubleValue;
        char      *StringValue;
        char       OffendingChar;
    };
} TOKEN;

typedef enum {
    LM_DEFAULT             = 0,
    LM_RAW                 = 1,
    LM_PP_DIRECTIVE        = 2,
    LM_PP_DIRECTIVE_KW     = 4,
    LM_ANGLED_STR_CONSTANT = 8
} LEXER_MODE;

typedef struct LEXER LEXER;

extern LEXER *CreateLexer(SOURCE_FILE *input);
extern void   DeleteLexer(LEXER *l);
extern void   EnableLexerMode(LEXER *l, LEXER_MODE modes);
extern void   DisableLexerMode(LEXER *l, LEXER_MODE modes);
extern TOKEN  PeekTokenDirect(LEXER *l);
extern TOKEN  ReadTokenDirect(LEXER *l);
extern void   FreeToken(TOKEN *t);

#endif
