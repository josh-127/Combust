#include "syntax.h"
#include <stdlib.h>

static BOOL g_tokenMask[] = {
#define Token(name) TRUE,
#define Node(name) FALSE
#include "syntax-kinds.def"
#undef Node
#undef Token
};

void DeleteSyntaxNode(
    THIS PSYNTAX_NODE obj
)
{
    if (g_tokenMask[obj->Kind]) {
        PSYNTAX_TOKEN token = (PSYNTAX_TOKEN) obj;

        if (token->Base.Kind == SK_IDENTIFIER_TOKEN)
            free(token->Value.IdentifierName);
        else if (token->Base.Kind == SK_STRING_CONSTANT_TOKEN)
            free(token->Value.StringValue);
    }
}
