#ifndef PTC_CL_LEXER_H
#define PTC_CL_LEXER_H
#include "common.h"
#include "source.h"
#include "syntax.h"

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
