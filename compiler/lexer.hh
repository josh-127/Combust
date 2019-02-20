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

    std::shared_ptr<SyntaxToken> ReadTokenDirect();

private:
    char Peek(int index = 0);
    std::tuple<char, int> DecodeTrigraph();
    std::tuple<char, int, int> DecodeNewLineEscape();
    std::tuple<char, int> GetCharEx();
    char GetChar();
    void IncrementCursor();
    void IncrementCursorBy(IN int amount);
    void GetTokenRange(
        const std::shared_ptr<SyntaxToken> t,
        OUT   PSOURCE_RANGE range
    ) noexcept;
    void ReadIdentifier(std::shared_ptr<SyntaxToken> t);
    std::string ReadSuffix();
    void SkipIntSuffixes(const std::shared_ptr<SyntaxToken> t);
    void ReadFractionalLiteral(std::shared_ptr<SyntaxToken> t);
    void ReadHexLiteral(std::shared_ptr<SyntaxToken> t);
    void ReadOctalLiteral(std::shared_ptr<SyntaxToken> t);
    void ReadDecimalLiteral(std::shared_ptr<SyntaxToken> t);
    void ReadNumericalLiteral(std::shared_ptr<SyntaxToken> t);
    int ReadCharEscapeSequence();
    void ReadCharLiteral(std::shared_ptr<SyntaxToken> t);
    void ReadStringLiteral(std::shared_ptr<SyntaxToken> t);
    std::shared_ptr<SyntaxToken> ReadTokenOnce();

private:
    std::unique_ptr<LEXER_IMPL> l;
};

#endif
