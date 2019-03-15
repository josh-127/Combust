#ifndef COMBUST_PREPROCESSOR_HH
#define COMBUST_PREPROCESSOR_HH
#include "common.hh"

class CodeLexer;
class SourceFile;
class SyntaxToken;

struct PREPROCESSOR_IMPL;

class Preprocessor : public Object {
public:
    explicit Preprocessor(Rc<const SourceFile> input);
    virtual ~Preprocessor();

    Rc<SyntaxToken> ReadToken();

private:
    Rc<SyntaxToken> ReadToken_Internal();

private:
    Owner<CodeLexer> lexer;
    Owner<PREPROCESSOR_IMPL> p;
};

#endif
