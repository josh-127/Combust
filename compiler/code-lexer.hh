#ifndef COMBUST_CODE_LEXER_HH
#define COMBUST_CODE_LEXER_HH
#include "common.hh"
#include "lexer.hh"
#include "source.hh"
#include "syntax.hh"
#include <memory>
#include <string>
#include <tuple>

struct CODE_LEXER_IMPL;

class CodeLexer : public Object, public virtual ILexer {
public:
    explicit CodeLexer(Rc<const SourceFile> input);
    virtual ~CodeLexer();

    Rc<SyntaxToken> ReadToken() override;
    char PeekChar() const;
    char ReadChar();
    bool IsAtBeginningOfLine() const;
    const SourceLoc& GetCurrentLocation() const;

    Rc<SyntaxToken> ReadIdentifierOrKeyword();
    Rc<StringLiteralToken> ReadStringLiteral(
        const char openingQuote,
        const char closingQuote
    );
    Rc<CommentToken> ReadComment();

private:
    char Peek(int index = 0) const;
    std::tuple<char, int> DecodeTrigraph() const;
    std::tuple<char, int, bool> DecodeNewLineEscape() const;
    std::tuple<char, int> GetCharEx() const;
    char GetChar() const;
    void IncrementCursor();
    void IncrementCursorBy(IN int amount);
    SourceRange GetTokenRange(const Rc<SyntaxToken> t);
    Rc<SyntaxToken> ReadIdentifierOrKeyword_Internal();
    Rc<NumericLiteralToken> ReadHexLiteral_Internal();
    Rc<NumericLiteralToken> ReadDecimalOrOctalLiteral_Internal();
    Rc<NumericLiteralToken> ReadNumericLiteral_Internal();
    Rc<StringLiteralToken> ReadStringLiteral_Internal(
        const char openingQuote,
        const char closingQuote
    );
    Rc<CommentToken> ReadComment_Internal();
    Rc<SyntaxToken> ReadTokenOnce();

private:
    Owner<CODE_LEXER_IMPL> l;
};

#endif
