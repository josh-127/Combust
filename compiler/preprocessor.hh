#ifndef COMBUST_PREPROCESSOR_HH
#define COMBUST_PREPROCESSOR_HH
#include "common.hh"

class Lexer;
class SourceFile;
class SyntaxToken;

class Preprocessor : public Object {
public:
    explicit Preprocessor(Rc<const SourceFile> input);
    virtual ~Preprocessor();

    Rc<SyntaxToken> ReadToken();

private:
    Owner<Lexer> lexer;
};

#endif
