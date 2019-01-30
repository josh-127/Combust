#ifndef PTC_CL_LEXER_H
#define PTC_CL_LEXER_H
#include "common.hh"
#include "source.hh"
#include "syntax.hh"

typedef struct tagLexer Lexer, *PLEXER;

PLEXER CreateLexer(
    IN PSOURCE_FILE input
);

void DeleteLexer(
    THIS PLEXER l
);

PSYNTAX_TOKEN ReadTokenDirect(
    THIS PLEXER l
);

#endif
