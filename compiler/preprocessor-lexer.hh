#ifndef COMBUST_PREPROCESSOR_LEXER_HH
#define COMBUST_PREPROCESSOR_LEXER_HH
#include "common.hh"
#include "lexer.hh"

class CodeLexer;
class SourceFile;
class SyntaxToken;

struct PREPROCESSOR_LEXER_IMPL;

class PreprocessorLexer : public Object, public virtual ILexer {
public:
    explicit PreprocessorLexer(Rc<const SourceFile> input);
    virtual ~PreprocessorLexer();

    Rc<SyntaxToken> ReadToken() override;

private:
    Rc<SyntaxToken> ReadToken_Internal();

private:
    Owner<CodeLexer> lexer;
    Owner<PREPROCESSOR_LEXER_IMPL> p;
};

#endif
