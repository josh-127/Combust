#ifndef PTC_CL_LEXER_H
#define PTC_CL_LEXER_H
#include "common.hh"
#include "source.hh"
#include "syntax.hh"
#include <memory>
#include <tuple>

struct LEXER_IMPL;

class Lexer {
public:
    Lexer(IN SourceFile* input);
    virtual ~Lexer();

    SYNTAX_TOKEN ReadTokenDirect();

private:
    char Peek(int index = 0);
    std::tuple<char, int> DecodeTrigraph();
    std::tuple<char, int, int> DecodeNewLineEscape();
    std::tuple<char, int> GetCharEx();
    char GetChar();
    void IncrementCursor();
    void IncrementCursorBy(IN int amount);
    void GetTokenRange(
        IN  PSYNTAX_TOKEN t,
        OUT PSOURCE_RANGE range
    ) noexcept;
    void ReadIdentifier(OUT PSYNTAX_TOKEN t);
    void ReadSuffix(
        OUT char** suffix,
        OUT int*   length
    );
    int SkipUnsignedSuffix(IN_OUT char** cursor);
    int SkipLongSuffix(IN_OUT char** cursor);
    void SkipIntSuffixes(IN PSYNTAX_TOKEN t);
    void ReadFractionalLiteral(IN PSYNTAX_TOKEN t);
    void ReadHexLiteral(OUT PSYNTAX_TOKEN t);
    void ReadOctalLiteral(OUT PSYNTAX_TOKEN t);
    void ReadDecimalLiteral(OUT PSYNTAX_TOKEN t);
    void ReadNumericalLiteral(OUT PSYNTAX_TOKEN t);
    int ReadCharEscapeSequence();
    void ReadCharLiteral(OUT PSYNTAX_TOKEN t);
    void ReadStringLiteral(OUT PSYNTAX_TOKEN t);
    SYNTAX_TOKEN ReadTokenOnce();

private:
    std::unique_ptr<LEXER_IMPL> l;
};

#endif
