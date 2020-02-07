#ifndef COMBUST_LEXER_HH
#define COMBUST_LEXER_HH
#include "common.hh"

class SyntaxToken;

class ILexer {
public:
    virtual Rc<SyntaxToken> ReadToken() = 0;
};

#endif
