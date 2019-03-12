#ifndef COMBUST_LEXER_HH
#define COMBUST_LEXER_HH
#include "common.hh"
#include "source.hh"
#include "syntax.hh"
#include <memory>
#include <string>
#include <tuple>

struct LEXER_IMPL;

class Lexer : public Object {
public:
    explicit Lexer(Rc<const SourceFile> input);
    virtual ~Lexer();

    Rc<SyntaxToken> ReadTokenDirect();

private:
    char Peek(int index = 0);
    std::tuple<char, int> DecodeTrigraph();
    std::tuple<char, int, int> DecodeNewLineEscape();
    std::tuple<char, int> GetCharEx();
    char GetChar();
    void IncrementCursor();
    void IncrementCursorBy(IN int amount);
    SourceRange GetTokenRange(const Rc<SyntaxToken> t);
    Rc<SyntaxToken> ReadIdentifier();
    std::string ReadSuffix();
    Rc<SyntaxToken> ReadFractionalLiteral(const Rc<IntConstantToken> t);
    Rc<IntConstantToken> ReadHexLiteral();
    Rc<IntConstantToken> ReadOctalLiteral();
    Rc<SyntaxToken> ReadDecimalLiteral();
    Rc<SyntaxToken> ReadNumericalLiteral();
    int ReadCharEscapeSequence();
    Rc<SyntaxToken> ReadCharLiteral();
    Rc<SyntaxToken> ReadStringLiteral();
    Rc<SyntaxToken> ReadTokenOnce();

private:
    std::unique_ptr<LEXER_IMPL> l;
};

#endif
