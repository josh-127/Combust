#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "lexer.hh"
#include "logger.hh"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#define LM_DEFAULT                   0
#define LM_PP_DIRECTIVE              1
#define LM_PP_DIRECTIVE_KW           2
#define LM_PP_ANGLED_STRING_CONSTANT 4

struct LEXER_IMPL {
    Rc<const SourceFile> Source{ nullptr };
    int                  Cursor{ 0 };
    uint32_t             CurrentMode{ 0 };
    int                  CurrentFlags{ 0 };
    SourceLoc            CurrentLocation{ };
    Rc<SyntaxToken>      CurrentToken{ };
};

Lexer::Lexer(Rc<const SourceFile> input) :
    l{ std::make_unique<LEXER_IMPL>() }
{
    l->Source                 = input;
    l->CurrentMode            = LM_DEFAULT;
    l->CurrentFlags           = ST_BEGINNING_OF_LINE;
    l->CurrentLocation.Source = l->Source;
}

Lexer::~Lexer() {}

Rc<SyntaxToken> Lexer::ReadTokenDirect() {
    for (;;) {
        Rc<SyntaxToken> token{ ReadTokenOnce() };
        l->CurrentToken = token;

        return token;
    }
}

constexpr bool IsWhitespace(char c) noexcept {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}
constexpr bool IsLetter(char c) noexcept {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}
constexpr bool IsDecimal(char c) noexcept { return c >= '0' && c <= '9'; }
constexpr bool IsOctal(char c) noexcept { return c >= '0' && c <= '7'; }
constexpr bool IsHex(char c) noexcept {
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

char Lexer::Peek(int index) {
    return l->Source->Contents[l->Cursor + index];
}

/**
 * Decodes a trigraph sequence (see ANSI C Draft: 2.2.1.1).
 * If the current sequence is not a trigraph, this function returns a
 * normal character.
 * \return first value as the decoded character;
 *         second value as the encoded sequence's length
 */
std::tuple<char, int> Lexer::DecodeTrigraph() {
    if (Peek(0) == '?' && Peek(1) == '?') {
        switch (Peek(2)) {
        case '=':  return std::make_tuple('#', 3);
        case '(':  return std::make_tuple('[', 3);
        case '/':  return std::make_tuple('\\', 3);
        case ')':  return std::make_tuple(']', 3);
        case '\'': return std::make_tuple('^', 3);
        case '<':  return std::make_tuple('{', 3);
        case '!':  return std::make_tuple('|', 3);
        case '>':  return std::make_tuple('}', 3);
        case '-':  return std::make_tuple('~', 3);
        }
    }
    return std::make_tuple(Peek(), 1);
}

/**
 * Performs the same routines as DecodeTrigraph, and then decodes
 * new-line escapes ("\") into a single space character.
 * \return first value as the decoded character;
 *         second value as the encoded sequence's length;
 *         third value is true if the the sequence is a new-line escape;
 *                     otherwise false
 */
std::tuple<char, int, bool> Lexer::DecodeNewLineEscape() {
    auto [firstChar, firstCharLength] = DecodeTrigraph();

    if (firstChar == '\\') {
        bool isOnlyWhitespace{ true };
        int length{ firstCharLength };

        while (Peek(length) != '\n') {
            if (!IsWhitespace(Peek(length))) {
                isOnlyWhitespace = false;
                break;
            }
            ++length;
        }

        if (isOnlyWhitespace) {
            return std::make_tuple(' ', length + 1, true);
        }
    }

    return std::make_tuple(firstChar, firstCharLength, false);
}

/**
 * Decodes the current character sequence.
 * \return first value as the decoded character;
 *         second value as the encoded sequence length
 */
std::tuple<char, int> Lexer::GetCharEx() {
    std::tuple<char, int, bool> values{ DecodeNewLineEscape() };
    return std::make_tuple(std::get<0>(values), std::get<1>(values));
}

/**
 * Decodes the current character sequence.
 * \return the decoded character
 */
char Lexer::GetChar() {
    return std::get<0>(GetCharEx());
}

void Lexer::IncrementCursor() {
    auto [charValue, charLength, isNewLineEscape] = DecodeNewLineEscape();

    // New-Line
    if (charValue == '\n') {
        ++l->CurrentLocation.Line;
        l->CurrentLocation.Column = 0;
        l->CurrentFlags |= ST_BEGINNING_OF_LINE;
    }
    // New-Line Escape Sequence
    else if (isNewLineEscape) {
        ++l->CurrentLocation.Line;
        l->CurrentLocation.Column = 0;
    }
    // Normal Character
    else {
        l->CurrentLocation.Column += charLength;

        if (!IsWhitespace(charValue))
            l->CurrentFlags &= ~ST_BEGINNING_OF_LINE;
    }

    l->Cursor += charLength;
}

void Lexer::IncrementCursorBy(IN   int    amount) {
    while (amount--)
        IncrementCursor();
}

SourceRange Lexer::GetTokenRange(const Rc<SyntaxToken> t) {
    SourceRange range{ };

    int line{ t->GetLexemeRange().Location.Line };
    int column{ t->GetLexemeRange().Location.Column };

    // TODO: Re-implement length calculation.
    range.Location.Source = l->Source;
    range.Location.Line = line;
    range.Location.Column = column;
    range.Length = 1;

    return range;
}

/* Precondition: GetChar() is [_A-Za-z] */
Rc<SyntaxToken> Lexer::ReadIdentifier() {
    Rc<SyntaxToken> result{ };
    std::string name{ };

    for (;;) {
        char character{ GetChar() };

        if ((character == '_') ||
            (character == '$') ||
            (character >= 'A' && character <= 'Z') ||
            (character >= 'a' && character <= 'z') ||
            (character >= '0' && character <= '9'))
        {
            name += character;
            IncrementCursor();
        }
        else {
            break;
        }
    }

#define o(kw) (name == kw)
    if (l->CurrentMode & LM_PP_DIRECTIVE_KW) {
             if (o("if"))       result = NewObj<IfDirectiveKw>();
        else if (o("ifdef"))    result = NewObj<IfDefDirectiveKw>();
        else if (o("ifndef"))   result = NewObj<IfNDefDirectiveKw>();
        else if (o("elif"))     result = NewObj<ElifDirectiveKw>();
        else if (o("endif"))    result = NewObj<EndIfDirectiveKw>();
        else if (o("include"))  result = NewObj<IncludeDirectiveKw>();
        else if (o("define"))   result = NewObj<DefineDirectiveKw>();
        else if (o("undef"))    result = NewObj<UnDefDirectiveKw>();
        else if (o("line"))     result = NewObj<LineDirectiveKw>();
        else if (o("error"))    result = NewObj<ErrorDirectiveKw>();
        else if (o("warning"))  result = NewObj<WarningDirectiveKw>();
        else {
            result = NewObj<InvalidDirective>();
            name = "<invalid>";
        }
    }
    else if (o("const"))    result = NewObj<ConstKeyword>();
    else if (o("extern"))   result = NewObj<ExternKeyword>();
    else if (o("static"))   result = NewObj<StaticKeyword>();
    else if (o("auto"))     result = NewObj<AutoKeyword>();
    else if (o("volatile")) result = NewObj<VolatileKeyword>();
    else if (o("unsigned")) result = NewObj<UnsignedKeyword>();
    else if (o("signed"))   result = NewObj<SignedKeyword>();
    else if (o("void"))     result = NewObj<VoidKeyword>();
    else if (o("char"))     result = NewObj<CharKeyword>();
    else if (o("short"))    result = NewObj<ShortKeyword>();
    else if (o("int"))      result = NewObj<IntKeyword>();
    else if (o("long"))     result = NewObj<LongKeyword>();
    else if (o("float"))    result = NewObj<FloatKeyword>();
    else if (o("double"))   result = NewObj<DoubleKeyword>();
    else if (o("enum"))     result = NewObj<EnumKeyword>();
    else if (o("struct"))   result = NewObj<StructKeyword>();
    else if (o("union"))    result = NewObj<UnionKeyword>();
    else if (o("typedef"))  result = NewObj<TypeDefKeyword>();
    else if (o("sizeof"))   result = NewObj<SizeOfKeyword>();
    else if (o("register")) result = NewObj<RegisterKeyword>();
    else if (o("goto"))     result = NewObj<GotoKeyword>();
    else if (o("if"))       result = NewObj<IfKeyword>();
    else if (o("else"))     result = NewObj<ElseKeyword>();
    else if (o("switch"))   result = NewObj<SwitchKeyword>();
    else if (o("case"))     result = NewObj<CaseKeyword>();
    else if (o("default"))  result = NewObj<DefaultKeyword>();
    else if (o("do"))       result = NewObj<DoKeyword>();
    else if (o("while"))    result = NewObj<WhileKeyword>();
    else if (o("for"))      result = NewObj<ForKeyword>();
    else if (o("break"))    result = NewObj<BreakKeyword>();
    else if (o("continue")) result = NewObj<ContinueKeyword>();
    else if (o("return"))   result = NewObj<ReturnKeyword>();
    else {
        Rc<IdentifierToken> identifier{ NewObj<IdentifierToken>() };
        identifier->SetName(name);
        result = identifier;
    }
#undef o

    return result;
}

Rc<NumericLiteralToken> Lexer::ReadHexLiteral() {
    Rc<NumericLiteralToken> result{ NewObj<NumericLiteralToken>() };
    std::string wholeValue{ };
    std::string suffix{ };

    while (IsHex(GetChar())) {
        wholeValue += GetChar();
        IncrementCursor();
    }

    while (IsLetter(GetChar())) {
        suffix += GetChar();
        IncrementCursor();
    }

    result->SetWholeValue(wholeValue);
    result->SetSuffix(suffix);

    return result;
}

Rc<NumericLiteralToken> Lexer::ReadDecimalOrOctalLiteral() {
    Rc<NumericLiteralToken> result{ NewObj<NumericLiteralToken>() };
    std::string wholeValue{ };
    std::string fractionalValue{ };
    std::string suffix{ };

    while (IsDecimal(GetChar())) {
        wholeValue += GetChar();
        IncrementCursor();
    }

    if (GetChar() == '.') {
        result->SetDotSymbol(".");

        while (IsDecimal(GetChar())) {
            fractionalValue += GetChar();
            IncrementCursor();
        }
    }

    if (wholeValue.size() == 0 && fractionalValue.size() == 0)
        return Rc<NumericLiteralToken>{ };

    while (IsLetter(GetChar())) {
        suffix += GetChar();
        IncrementCursor();
    }

    result->SetWholeValue(wholeValue);
    result->SetFractionalValue(fractionalValue);
    result->SetSuffix(suffix);

    return result;
}

Rc<NumericLiteralToken> Lexer::ReadNumericLiteral() {
    if (Peek(0) == '0' && (Peek(1) == 'X' || Peek(1) == 'x')) {
        return ReadHexLiteral();
    }
    else {
        return ReadDecimalOrOctalLiteral();
    }
}

Rc<StringLiteralToken> Lexer::ReadStringLiteral(
    const char openingQuote,
    const char closingQuote
) {
    if (GetChar() != openingQuote)
        return Rc<StringLiteralToken>{ };

    Rc<StringLiteralToken> result{ NewObj<StringLiteralToken>() };
    result->SetOpeningQuote(GetChar());
    IncrementCursor();

    std::string value{ };

    while (GetChar() != closingQuote && GetChar() != '\n') {
        value += GetChar();
        IncrementCursor();
    }

    result->SetClosingQuote(GetChar());
    IncrementCursor();
    return result;
}

Rc<SyntaxToken> Lexer::ReadTokenOnce() {
    Rc<SyntaxToken> result{ };

    while (IsWhitespace(GetChar()))
        IncrementCursor();

    if (l->CurrentFlags & ST_BEGINNING_OF_LINE)
        l->CurrentMode = LM_DEFAULT;

    int flags{ l->CurrentFlags };
    SourceRange lexemeRange{ };
    lexemeRange.Location = l->CurrentLocation;
    lexemeRange.Length = 1;

    if ((flags & ST_BEGINNING_OF_LINE) && GetChar() == '#') {
        IncrementCursor();
        result = NewObj<HashSymbol>();
    }
    else {
        switch (GetChar()) {
        case 0:                      result = NewObj<EofToken>(); break;
        case '(': IncrementCursor(); result = NewObj<LParenSymbol>(); break;
        case ')': IncrementCursor(); result = NewObj<RParenSymbol>(); break;
        case '[': IncrementCursor(); result = NewObj<LBracketSymbol>(); break;
        case ']': IncrementCursor(); result = NewObj<RBracketSymbol>(); break;
        case '{': IncrementCursor(); result = NewObj<LBraceSymbol>(); break;
        case '}': IncrementCursor(); result = NewObj<RBraceSymbol>(); break;
        case ';': IncrementCursor(); result = NewObj<SemicolonSymbol>(); break;
        case ',': IncrementCursor(); result = NewObj<CommaSymbol>(); break;
        case '~': IncrementCursor(); result = NewObj<TildeSymbol>(); break;
        case '?': IncrementCursor(); result = NewObj<QuestionSymbol>(); break;
        case ':': IncrementCursor(); result = NewObj<ColonSymbol>(); break;

        case '.': {
            Rc<NumericLiteralToken> literal{ ReadNumericLiteral() };
            if (literal == nullptr) {
                result = NewObj<DotSymbol>();
            }
            else {
                result = literal;
            }
            break;
        }

        case '+':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result = NewObj<PlusEqualsSymbol>(); }
            else if (GetChar() == '+') { IncrementCursor(); result = NewObj<PlusPlusSymbol>(); }
            else { result = NewObj<PlusSymbol>(); }
            break;

        case '-':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result = NewObj<MinusEqualsSymbol>(); }
            else if (GetChar() == '-') { IncrementCursor(); result = NewObj<MinusMinusSymbol>(); }
            else if (GetChar() == '>') { IncrementCursor(); result = NewObj<MinusGtSymbol>(); }
            else { result = NewObj<MinusSymbol>(); }
            break;

        case '*':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result = NewObj<AsteriskEqualsSymbol>(); }
            else { result = NewObj<AsteriskSymbol>(); }
            break;

        case '/':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result = NewObj<SlashEqualsSymbol>();
            }
            else if (GetChar() == '*') {
                IncrementCursor();
                result = NewObj<CommentToken>();

                for (;;) {
                    if (GetChar() == '*') {
                        IncrementCursor();

                        if (GetChar() == '/') {
                            IncrementCursor();
                            break;
                        }
                        else if (GetChar() == 0) {
                            SourceRange range{ GetTokenRange(result) };
                            LogAtRange(
                                &range,
                                LL_ERROR,
                                "unterminated comment"
                            );
                            break;
                        }
                    }
                    else if (GetChar() == 0) {
                        SourceRange range{ GetTokenRange(result) };
                        LogAtRange(
                            &range,
                            LL_ERROR,
                            "unterminated comment"
                        );
                        break;
                    }
                    else {
                        IncrementCursor();
                    }
                }
            }
            else {
                result = NewObj<SlashSymbol>();
            }
            break;

        case '%':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result = NewObj<PercentEqualsSymbol>(); }
            else { result = NewObj<PercentSymbol>(); }
            break;

        case '<':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result = NewObj<LtEqualsSymbol>();
            }
            else if (GetChar() == '<') {
                IncrementCursor();
                if (GetChar() == '=') {
                    IncrementCursor();
                    result = NewObj<LtLtEqualsSymbol>();
                }
                else {
                    result = NewObj<LtLtSymbol>();
                }
            }
            else {
                result = NewObj<LtSymbol>();
            }
            break;

        case '>':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result = NewObj<GtEqualsSymbol>();
            }
            else if (GetChar() == '>') {
                IncrementCursor();
                if (GetChar() == '=') {
                    IncrementCursor();
                    result = NewObj<GtGtEqualsSymbol>();
                }
                else {
                    result = NewObj<GtGtSymbol>();
                }
            }
            else {
                result = NewObj<GtSymbol>();
            }
            break;

        case '=':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result = NewObj<EqualsEqualsSymbol>(); }
            else { result = NewObj<EqualsSymbol>(); }
            break;

        case '!':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result = NewObj<ExclamationEqualsSymbol>(); }
            else { result = NewObj<ExclamationSymbol>(); }
            break;

        case '&':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result = NewObj<AmpersandEqualsSymbol>(); }
            else if (GetChar() == '&') { IncrementCursor(); result = NewObj<AmpersandAmpersandSymbol>(); }
            else { result = NewObj<AmpersandSymbol>(); }
            break;

        case '^':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result = NewObj<CaretEqualsSymbol>(); }
            else { result = NewObj<CaretSymbol>(); } 
            break;

        case '|':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result = NewObj<PipeEqualsSymbol>(); }
            else if (GetChar() == '|') { IncrementCursor(); result = NewObj<PipePipeSymbol>(); }
            else { result = NewObj<PipeSymbol>(); }
            break;

        case '_': case '$':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
        case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
        case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z': 
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
        case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
        case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
        case 'y': case 'z':
            result = ReadIdentifier();
            break;

        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            result = ReadNumericLiteral();
            break;

        case '\'': result = ReadStringLiteral('\'', '\''); break;
        case '"': result = ReadStringLiteral('"', '"'); break;

        default:
            Rc<StrayToken> strayToken{ NewObj<StrayToken>() };
            strayToken->SetOffendingChar(GetChar());
            IncrementCursor();

            result = strayToken;
            break;
        }
    }

    result->SetLexemeRange(lexemeRange);

    if (l->CurrentMode == LM_DEFAULT && IsToken<HashSymbol>(result)) {
        l->CurrentMode |= LM_PP_DIRECTIVE;
        l->CurrentMode |= LM_PP_DIRECTIVE_KW;
    }
    else if (l->CurrentMode & LM_PP_DIRECTIVE_KW) {
        l->CurrentMode &= ~LM_PP_DIRECTIVE_KW;

        if (IsToken<IncludeDirectiveKw>(result))
            l->CurrentMode |= LM_PP_ANGLED_STRING_CONSTANT;
    }
    else if (l->CurrentMode & LM_PP_ANGLED_STRING_CONSTANT) {
        l->CurrentMode &= ~LM_PP_ANGLED_STRING_CONSTANT;
    }

    result->SetFlags(l->CurrentFlags);
    return result;
}
