#ifndef COMBUST_PREPROCESSOR_LEXER_HH
#define COMBUST_PREPROCESSOR_LEXER_HH
#include "common.hh"

class CodeLexer;
class SourceFile;
class SyntaxToken;

struct PREPROCESSOR_LEXER_IMPL;

class PreprocessorLexer : public Object {
public:
    explicit PreprocessorLexer(Rc<const SourceFile> input);
    virtual ~PreprocessorLexer();

    Rc<SyntaxToken> ReadToken();

private:
    Rc<SyntaxToken> ReadToken_Internal();

private:
    Owner<CodeLexer> lexer;
    Owner<PREPROCESSOR_LEXER_IMPL> p;
};

#endif
