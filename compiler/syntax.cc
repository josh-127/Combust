#include "syntax.hh"
#include <stdlib.h>

static bool g_tokenMask[] = {
#define Token(name) true,
#define Node(name) false,
#include "syntax-kinds.def"
#undef Node
#undef Token
};

void DeleteSyntaxNode(
    THIS PSYNTAX_NODE obj
)
{
    if (g_tokenMask[obj->Kind]) {
        PSYNTAX_TOKEN token{ (PSYNTAX_TOKEN) obj };

        if (token->Base.Kind == SK_IDENTIFIER_TOKEN)
            delete[] token->Value.IdentifierName;
        else if (token->Base.Kind == SK_STRING_CONSTANT_TOKEN)
            delete[] token->Value.StringValue;
    }
}
