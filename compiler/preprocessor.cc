#include "preprocessor.hh"
#include "lexer.hh"
#include "source.hh"
#include "syntax.hh"

Preprocessor::Preprocessor(Rc<const SourceFile> input) :
    lexer{ NewChild<Lexer>(input) }
{}

Preprocessor::~Preprocessor() {}

Rc<SyntaxToken> Preprocessor::ReadToken() {
    return NewObj<EofToken>();
}