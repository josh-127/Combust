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
    SourceFile*     Source{ nullptr };
    int             Cursor{ 0 };
    uint32_t        CurrentMode{ 0 };
    int             CurrentFlags{ 0 };
    SOURCE_LOC      CurrentLocation{ };
    Rc<SyntaxToken> CurrentToken{ };
};

Lexer::Lexer(IN SourceFile* input) :
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
        auto [token, asStrayToken, isCommentToken, isEofToken] = ReadTokenOnce();
        l->CurrentToken = token;

        if (asStrayToken) {
            SOURCE_RANGE range{ l->CurrentToken->GetLexemeRange() };
            LogAtRange(
                &range,
                LL_ERROR,
                "stray '%c' in program",
                asStrayToken->GetOffendingChar()
            );
        }
        else if (isEofToken) {
            return Rc<SyntaxToken>{ };
        }
        else if (isCommentToken) {
            return l->CurrentToken;
        }
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

std::tuple<char, int, int> Lexer::DecodeNewLineEscape() {
    auto [firstChar, firstCharLength] = DecodeTrigraph();

    if (firstChar == '\\') {
        bool isOnlyWhitespace{ true };
        int lengthWithoutNewLine{ firstCharLength };

        while (Peek(lengthWithoutNewLine) != '\n') {
            if (!IsWhitespace(Peek(lengthWithoutNewLine))) {
                isOnlyWhitespace = false;
                break;
            }
            ++lengthWithoutNewLine;
        }

        if (isOnlyWhitespace) {
            int totalLength{ lengthWithoutNewLine + 1 };
            int trailingWhitespaceLength{ lengthWithoutNewLine - firstCharLength };
            return std::make_tuple(' ', totalLength, trailingWhitespaceLength);
        }
    }

    return std::make_tuple(firstChar, firstCharLength, -1);
}

std::tuple<char, int> Lexer::GetCharEx() {
    std::tuple<char, int, int> values{ DecodeNewLineEscape() };
    return std::make_tuple(std::get<0>(values), std::get<1>(values));
}

char Lexer::GetChar() {
    return std::get<0>(GetCharEx());
}

void Lexer::IncrementCursor() {
    auto [charValue, charLength, trailingWhitespaceLength] = DecodeNewLineEscape();

    if (charValue == '\n') {
        ++l->CurrentLocation.Line;
        l->CurrentLocation.Column = 0;
        l->CurrentFlags |= ST_BEGINNING_OF_LINE;
    }
    else if (trailingWhitespaceLength < 0) {
        l->CurrentLocation.Column += charLength;

        if (!IsWhitespace(charValue))
            l->CurrentFlags &= ~ST_BEGINNING_OF_LINE;
    }
    else {
        if (trailingWhitespaceLength > 0) {
            LogAt(
                &l->CurrentLocation,
                LL_WARNING,
                "backslash and newline separated by space"
            );
        }

        ++l->CurrentLocation.Line;
        l->CurrentLocation.Column = 0;
    }

    l->Cursor += charLength;
}

void Lexer::IncrementCursorBy(IN   int    amount) {
    while (amount--)
        IncrementCursor();
}

void Lexer::GetTokenRange(
    const Rc<SyntaxToken> t,
    OUT   PSOURCE_RANGE range
) noexcept {
    int line{ t->GetLexemeRange().Location.Line };
    int column{ t->GetLexemeRange().Location.Column };

    // TODO: Re-implement length calculation.
    range->Location.Source = l->Source;
    range->Location.Line = line;
    range->Location.Column = column;
    range->Length = 1;
}

/* Precondition: GetChar() is [_A-Za-z] */
std::tuple<Rc<SyntaxToken>, std::string> Lexer::ReadIdentifier() {
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

    return std::make_tuple(result, name);
}

std::string Lexer::ReadSuffix() {
    std::string suffix{ };
    for (char c{ GetChar() }; IsLetter(c); c = GetChar()) {
        suffix += c;
        IncrementCursor();
    }
    return suffix;
}

void Lexer::SkipIntSuffixes(const Rc<SyntaxToken> t) {
    std::string suffix{ ReadSuffix() };
    std::string ciSuffix{ suffix };
    std::transform(ciSuffix.begin(), ciSuffix.end(), ciSuffix.begin(), toupper);

    if (ciSuffix.length() != 0 &&
        ciSuffix != "UL" &&
        ciSuffix != "LU" &&
        ciSuffix != "ULL" &&
        ciSuffix != "LLU")
    {
        SOURCE_RANGE range;
        GetTokenRange(t, &range);

        LogAtRange(
            &range,
            LL_ERROR,
            "invalid suffix \"%s\" on integer constant",
            suffix.c_str()
        );
    }
}

Rc<SyntaxToken> Lexer::ReadFractionalLiteral(const Rc<IntConstantToken> t) {
    float floatFrac{ 0.0F };
    float floatExp{ 0.1F };
    double doubleFrac{ 0.0 };
    double doubleExp{ 0.1 };

    for (; IsDecimal(GetChar()); IncrementCursor()) {
        floatFrac += floatExp * (GetChar() - '0');
        doubleFrac += doubleExp * (GetChar() - '0');
        floatExp *= 0.1F;
        doubleExp *= 0.1;
    }

    std::string suffix{ ReadSuffix() };

    if (suffix == "f" || suffix == "F") {
        Rc<FloatConstantToken> constant{ NewObj<FloatConstantToken>() };
        constant->SetValue(t->GetValue() + floatFrac);
        return constant;
    }
    else if (suffix.length() == 0) {
        Rc<DoubleConstantToken> constant{ NewObj<DoubleConstantToken>() };
        constant->SetValue(t->GetValue() + doubleFrac);
        return constant;
    }
    else {
        SOURCE_RANGE range;
        GetTokenRange(t, &range);

        LogAtRange(
            &range,
            LL_ERROR,
            "invalid suffix \"%s\" on floating constant",
            suffix.c_str()
        );

        return NewObj<FloatConstantToken>();
    }
}

Rc<IntConstantToken> Lexer::ReadHexLiteral() {
    Rc<IntConstantToken> result{ NewObj<IntConstantToken>() };
    result->SetValue(0);

    while (IsHex(GetChar())) {
        result->SetValue(result->GetValue() * 16);

        if (GetChar() <= '9')
            result->SetValue(result->GetValue() + GetChar() - '0');
        else if (GetChar() <= 'F')
            result->SetValue(result->GetValue() + GetChar() - 'A' + 10);
        else
            result->SetValue(result->GetValue() + GetChar() - 'a' + 10);

        IncrementCursor();
    }

    SkipIntSuffixes(result);
    return result;
}

Rc<IntConstantToken> Lexer::ReadOctalLiteral() {
    Rc<IntConstantToken> result{ NewObj<IntConstantToken>() };
    result->SetValue(0);

    for (; IsDecimal(GetChar()); IncrementCursor()) {
        if (IsOctal(GetChar())) {
            result->SetValue((result->GetValue() * 8) - (GetChar() - '0'));
        }
        else {
            SOURCE_RANGE range{ };
            GetTokenRange(result, &range);

            LogAtRange(
                &range,
                LL_ERROR,
                "invalid digit \"%c\" in octal constant",
                GetChar()
            );
        }
    }

    SkipIntSuffixes(result);
    return result;
}

Rc<SyntaxToken> Lexer::ReadDecimalLiteral() {
    Rc<IntConstantToken> result{ NewObj<IntConstantToken>() };
    result->SetValue(0);

    for (; IsDecimal(GetChar()); IncrementCursor())
        result->SetValue((result->GetValue() * 10) + (GetChar() - '0'));

    if (GetChar() == '.') {
        IncrementCursor();
        return ReadFractionalLiteral(result);
    }
    else {
        SkipIntSuffixes(result);
        return result;
    }
}

Rc<SyntaxToken> Lexer::ReadNumericalLiteral() {
    if (GetChar() == '0') {
        int wholeLength{ 0 };

        do {
            IncrementCursor();
        }
        while (GetChar() == '0');

        while (IsDecimal(Peek(wholeLength)))
            ++wholeLength;

        if (Peek(wholeLength) == '.') {
            return ReadDecimalLiteral();
        }
        else if (GetChar() == 'X' || GetChar() == 'x') {
            IncrementCursor();
            return ReadHexLiteral();
        }
        else {
            return ReadOctalLiteral();
        }
    }
    else {
        return ReadDecimalLiteral();
    }
}

int Lexer::ReadCharEscapeSequence() {
    int result{ 0 };

    if (GetChar() == '\\') {
        SOURCE_RANGE errorRange{ };
        int digitCount{ 0 };

        errorRange.Location = l->CurrentLocation;
        errorRange.Length = 2;

        IncrementCursor();

        switch (GetChar()) {
            case '\'': IncrementCursor(); result = '\''; break;
            case '"':  IncrementCursor(); result = '\"'; break;
            case '?':  IncrementCursor(); result = '\?'; break;
            case '\\': IncrementCursor(); result = '\\'; break;
            case 'a':  IncrementCursor(); result = '\a'; break;
            case 'b':  IncrementCursor(); result = '\b'; break;
            case 'f':  IncrementCursor(); result = '\f'; break;
            case 'n':  IncrementCursor(); result = '\n'; break;
            case 'r':  IncrementCursor(); result = '\r'; break;
            case 't':  IncrementCursor(); result = '\t'; break;
            case 'v':  IncrementCursor(); result = '\v'; break;

            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7':
                result = 0;
                digitCount = 0;

                while (digitCount < 3 && IsOctal(GetChar())) {
                    result = (result * 8) + (GetChar() - '0');
                    ++digitCount;
                    IncrementCursor();
                }

                break;

            case 'x':
                IncrementCursor();
                result = 0;

                for (; IsHex(GetChar()); IncrementCursor()) {
                    result *= 16;
                    if (GetChar() <= '9')
                        result += GetChar() - '0';
                    else if (GetChar() <= 'F')
                        result += GetChar() - 'A' + 10;
                    else if (GetChar() <= 'f')
                        result += GetChar() - 'a' + 10;
                }

                break;

            default:
                LogAtRange(
                    &errorRange,
                    LL_ERROR,
                    "unknown escape sequence: '\\%c'",
                    GetChar()
                );

                IncrementCursor();
                break;
        }
    }
    else {
        result = GetChar();
        IncrementCursor();
    }

    return result;
}

/* Precondition: GetChar() == '\'' */
Rc<SyntaxToken> Lexer::ReadCharLiteral() {
    Rc<IntConstantToken> result{ NewObj<IntConstantToken>() };
    SOURCE_RANGE range{ };

    IncrementCursor();

    if (GetChar() == '\n') {
        GetTokenRange(result, &range);
        LogAtRange(
            &range,
            LL_ERROR,
            "missing terminating ' character"
        );
    }
    else {
        result->SetValue(ReadCharEscapeSequence());

        if (GetChar() == '\'') {
            IncrementCursor();
        }
        else {
            while (GetChar() != '\'') {
                if (GetChar() == '\n') {
                    GetTokenRange(result, &range);
                    LogAtRange(
                        &range,
                        LL_ERROR,
                        "missing terminating ' character"
                    );
                    return result;
                }

                IncrementCursor();
            }

            IncrementCursor();

            GetTokenRange(result, &range);
            LogAtRange(
                &range,
                LL_WARNING,
                "character constant too long for its type"
            );
        }
    }

    return result;
}

/* Precondition: GetChar() == '"' */
Rc<SyntaxToken> Lexer::ReadStringLiteral() {
    IncrementCursor();

    Rc<StringConstantToken> result{ NewObj<StringConstantToken>() };
    result->SetValue("");

    while (GetChar() != '"') {
        if (GetChar() == '\n') {
            SOURCE_RANGE range{ };
            GetTokenRange(result, &range);

            LogAtRange(
                &range,
                LL_ERROR,
                "missing terminating \" character"
            );

            result->SetValue("");
            return result;
        }

        std::string newValue{ result->GetValue() };
        newValue += ReadCharEscapeSequence();
        result->SetValue(newValue);
    }

    IncrementCursor();
    return result;
}

std::tuple<Rc<SyntaxToken>, Rc<StrayToken>, bool, bool> Lexer::ReadTokenOnce() {
    Rc<SyntaxToken> result{ };
    Rc<StrayToken> asStrayToken{ };
    bool isHashSymbol{ false };
    bool isIncludeDirectiveKw{ false };
    bool isInvalidDirective{ false };
    bool isCommentToken{ false };
    bool isEofToken{ false };

    while (IsWhitespace(GetChar()))
        IncrementCursor();

    if (l->CurrentFlags & ST_BEGINNING_OF_LINE)
        l->CurrentMode = LM_DEFAULT;

    int flags{ l->CurrentFlags };
    SOURCE_RANGE lexemeRange{ };
    lexemeRange.Location = l->CurrentLocation;
    lexemeRange.Length = 1;

    if ((flags & ST_BEGINNING_OF_LINE) && GetChar() == '#') {
        IncrementCursor();
        result = NewObj<HashSymbol>();
        isHashSymbol = true;
    }
    else {
        switch (GetChar()) {
        case 0: result = NewObj<EofToken>(); isEofToken = true; break;
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

        case '.':
            IncrementCursor();
            if (IsDecimal(GetChar())) {
                Rc<IntConstantToken> zero{ NewObj<IntConstantToken>() };
                zero->SetValue(0);
                result = ReadFractionalLiteral(zero);
            }
            else { result = NewObj<DotSymbol>(); }
            break;

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
                isCommentToken = true;

                for (;;) {
                    SOURCE_RANGE range{ };

                    if (GetChar() == '*') {
                        IncrementCursor();

                        if (GetChar() == '/') {
                            IncrementCursor();
                            break;
                        }
                        else if (GetChar() == 0) {
                            GetTokenRange(result, &range);
                            LogAtRange(
                                &range,
                                LL_ERROR,
                                "unterminated comment"
                            );
                            break;
                        }
                    }
                    else if (GetChar() == 0) {
                        GetTokenRange(result, &range);
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
        case 'y': case 'z': {
            std::tuple<Rc<SyntaxToken>, std::string> tuple{ ReadIdentifier() };
            result = std::get<0>(tuple);
            if (std::get<1>(tuple) == "include")
                isIncludeDirectiveKw = true;
            else if (std::get<1>(tuple) == "<invalid>")
                isInvalidDirective = true;
            break;
        }

        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            result = ReadNumericalLiteral();
            break;

        case '\'': result = ReadCharLiteral(); break;
        case '"': result = ReadStringLiteral(); break;

        default:
            asStrayToken = NewObj<StrayToken>();
            asStrayToken->SetOffendingChar(GetChar());
            result = asStrayToken;
            IncrementCursor();
            break;
        }
    }

    result->SetLexemeRange(lexemeRange);

    if (l->CurrentMode == LM_DEFAULT && isHashSymbol) {
        l->CurrentMode |= LM_PP_DIRECTIVE;
        l->CurrentMode |= LM_PP_DIRECTIVE_KW;
    }
    else if (l->CurrentMode & LM_PP_DIRECTIVE_KW) {
        l->CurrentMode &= ~LM_PP_DIRECTIVE_KW;

        if (isIncludeDirectiveKw)
            l->CurrentMode |= LM_PP_ANGLED_STRING_CONSTANT;
        else if (isInvalidDirective) {
            SOURCE_RANGE range;
            GetTokenRange(result, &range);
            LogAtRange(&range, LL_ERROR, "invalid directive.");
        }
            
    }
    else if (l->CurrentMode & LM_PP_ANGLED_STRING_CONSTANT) {
        l->CurrentMode &= ~LM_PP_ANGLED_STRING_CONSTANT;
    }

    result->SetFlags(l->CurrentFlags);
    return std::make_tuple(result, asStrayToken, isCommentToken, isEofToken);
}
