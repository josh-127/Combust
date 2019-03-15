#include "preprocessor.hh"
#include "lexer.hh"
#include "source.hh"
#include "syntax.hh"
#include <string>

constexpr bool IsWhitespace(char c) noexcept {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}
constexpr bool IsLetter(char c) noexcept {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}
constexpr bool IsDecimal(char c) noexcept { return c >= '0' && c <= '9'; }
constexpr bool IsIdentifierOrKeyword(char c) noexcept {
    return IsLetter(c) || IsDecimal(c);
}

Preprocessor::Preprocessor(Rc<const SourceFile> input) :
    lexer{ NewChild<Lexer>(input) }
{}

Preprocessor::~Preprocessor() {}

Rc<SyntaxToken> Preprocessor::ReadToken() {
    while (IsWhitespace(lexer->PeekChar()))
        lexer->ReadChar();

    if (lexer->IsAtBeginningOfLine() && lexer->PeekChar() == '#') {
        Rc<SyntaxToken> result{ };

        SourceRange range{ };
        range.Location = lexer->GetCurrentLocation();

        lexer->ReadChar();

        std::string keyword{ };
        while (IsIdentifierOrKeyword(lexer->PeekChar())) {
            keyword += lexer->ReadChar();
        }

        if (keyword == "if") {
            result = NewObj<IfDirective>();
        }
        else if (keyword == "ifdef") {
            result = NewObj<IfDefDirective>();
        }
        else if (keyword == "ifndef") {
            result = NewObj<IfNDefDirective>();
        }
        else if (keyword == "elif") {
            result = NewObj<ElifDirective>();
        }
        else if (keyword == "endif") {
            result = NewObj<EndIfDirective>();
        }
        else if (keyword == "include") {
            result = NewObj<IncludeDirective>();
        }
        else if (keyword == "define") {
            result = NewObj<DefineDirective>();
        }
        else if (keyword == "undef") {
            result = NewObj<UnDefDirective>();
        }
        else if (keyword == "line") {
            result = NewObj<LineDirective>();
        }
        else if (keyword == "error") {
            result = NewObj<ErrorDirective>();
        }
        else if (keyword == "warning") {
            result = NewObj<WarningDirective>();
        }
        else {
            Rc<InvalidDirective> directive{ NewObj<InvalidDirective>() };
            directive->SetName(keyword);
            result = directive;
        }

        range.Length = lexer->GetCurrentLocation().Column - range.Location.Column;
        result->SetFlags(SyntaxToken::BEGINNING_OF_LINE);
        result->SetLexemeRange(range);

        return result;
    }
    else {
        return lexer->ReadToken();
    }
}