#ifndef PTC_CL_LEXER_H
#define PTC_CL_LEXER_H
#include "common.hh"
#include "source.hh"
#include "syntax.hh"
#include <memory>
#include <string>
#include <tuple>

struct LEXER_IMPL;

class Lexer {
public:
    Lexer(IN SourceFile* input);
    Lexer(const Lexer& obj) = delete;
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
    void GetTokenRange(
        const Rc<SyntaxToken> t,
        OUT   PSOURCE_RANGE range
    ) noexcept;
    void ReadIdentifier(Rc<SyntaxToken> t);
    std::string ReadSuffix();
    void SkipIntSuffixes(const Rc<SyntaxToken> t);
    void ReadFractionalLiteral(Rc<SyntaxToken> t);
    void ReadHexLiteral(Rc<SyntaxToken> t);
    void ReadOctalLiteral(Rc<SyntaxToken> t);
    void ReadDecimalLiteral(Rc<SyntaxToken> t);
    void ReadNumericalLiteral(Rc<SyntaxToken> t);
    int ReadCharEscapeSequence();
    void ReadCharLiteral(Rc<SyntaxToken> t);
    void ReadStringLiteral(Rc<SyntaxToken> t);
    Rc<SyntaxToken> ReadTokenOnce();

private:
    std::unique_ptr<LEXER_IMPL> l;
};

#endif
