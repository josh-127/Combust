#define _CRT_SECURE_NO_WARNINGS
#include "lexer.hh"
#include "logger.hh"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#define LM_DEFAULT                   0
#define LM_PP_DIRECTIVE              1
#define LM_PP_DIRECTIVE_KW           2
#define LM_PP_ANGLED_STRING_CONSTANT 4

struct LEXER_IMPL {
    SourceFile*  Source{ nullptr };
    int          Cursor{ 0 };
    uint32_t     CurrentMode{ 0 };
    int          CurrentFlags{ 0 };
    SOURCE_LOC   CurrentLocation{ };
    SyntaxToken  CurrentToken{ };
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

SyntaxToken Lexer::ReadTokenDirect() {
    for (;;) {
        l->CurrentToken = ReadTokenOnce();

        if (l->CurrentToken.GetKind() == SK_STRAY_TOKEN) {
            LogAtRange(
                &l->CurrentToken.GetLexemeRange(),
                LL_ERROR,
                "stray '%c' in program",
                l->CurrentToken.GetOffendingChar()
            );
        }
        else if (l->CurrentToken.GetKind() != SK_COMMENT_TOKEN) {
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
    const SyntaxToken& t,
    OUT   PSOURCE_RANGE range
) noexcept {
    int line{ t.GetLexemeRange().Location.Line };
    int column{ t.GetLexemeRange().Location.Column };
    const char *base{ &l->Source->Lines[line][column] };

    // TODO: Re-implement length calculation.
    range->Location = t.GetLexemeRange().Location;
    range->Length = 1;
    //range->Length = static_cast<int>(l->Cursor - base);
}

/* Precondition: GetChar() is [_A-Za-z] */
void Lexer::ReadIdentifier(SyntaxToken& t) {
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
             if (o("if"))       t.SetKind(SK_IF_DIRECTIVE_KEYWORD);
        else if (o("ifdef"))    t.SetKind(SK_IFDEF_DIRECTIVE_KEYWORD);
        else if (o("ifndef"))   t.SetKind(SK_IFNDEF_DIRECTIVE_KEYWORD);
        else if (o("elif"))     t.SetKind(SK_ELIF_DIRECTIVE_KEYWORD);
        else if (o("endif"))    t.SetKind(SK_ENDIF_DIRECTIVE_KEYWORD);
        else if (o("include"))  t.SetKind(SK_INCLUDE_DIRECTIVE_KEYWORD);
        else if (o("define"))   t.SetKind(SK_DEFINE_DIRECTIVE_KEYWORD);
        else if (o("undef"))    t.SetKind(SK_UNDEF_DIRECTIVE_KEYWORD);
        else if (o("line"))     t.SetKind(SK_LINE_DIRECTIVE_KEYWORD);
        else if (o("error"))    t.SetKind(SK_ERROR_DIRECTIVE_KEYWORD);
        else if (o("warning"))  t.SetKind(SK_WARNING_DIRECTIVE_KEYWORD);
    }
    else if (o("const"))    t.SetKind(SK_CONST_KEYWORD);
    else if (o("extern"))   t.SetKind(SK_EXTERN_KEYWORD);
    else if (o("static"))   t.SetKind(SK_STATIC_KEYWORD);
    else if (o("auto"))     t.SetKind(SK_AUTO_KEYWORD);
    else if (o("volatile")) t.SetKind(SK_VOLATILE_KEYWORD);
    else if (o("unsigned")) t.SetKind(SK_UNSIGNED_KEYWORD);
    else if (o("signed"))   t.SetKind(SK_SIGNED_KEYWORD);
    else if (o("void"))     t.SetKind(SK_VOID_KEYWORD);
    else if (o("char"))     t.SetKind(SK_CHAR_KEYWORD);
    else if (o("short"))    t.SetKind(SK_SHORT_KEYWORD);
    else if (o("int"))      t.SetKind(SK_INT_KEYWORD);
    else if (o("long"))     t.SetKind(SK_LONG_KEYWORD);
    else if (o("float"))    t.SetKind(SK_FLOAT_KEYWORD);
    else if (o("double"))   t.SetKind(SK_DOUBLE_KEYWORD);
    else if (o("enum"))     t.SetKind(SK_ENUM_KEYWORD);
    else if (o("struct"))   t.SetKind(SK_STRUCT_KEYWORD);
    else if (o("union"))    t.SetKind(SK_UNION_KEYWORD);
    else if (o("typedef"))  t.SetKind(SK_TYPEDEF_KEYWORD);
    else if (o("sizeof"))   t.SetKind(SK_SIZEOF_KEYWORD);
    else if (o("register")) t.SetKind(SK_REGISTER_KEYWORD);
    else if (o("goto"))     t.SetKind(SK_GOTO_KEYWORD);
    else if (o("if"))       t.SetKind(SK_IF_KEYWORD);
    else if (o("else"))     t.SetKind(SK_ELSE_KEYWORD);
    else if (o("switch"))   t.SetKind(SK_SWITCH_KEYWORD);
    else if (o("case"))     t.SetKind(SK_CASE_KEYWORD);
    else if (o("default"))  t.SetKind(SK_DEFAULT_KEYWORD);
    else if (o("do"))       t.SetKind(SK_DO_KEYWORD);
    else if (o("while"))    t.SetKind(SK_WHILE_KEYWORD);
    else if (o("for"))      t.SetKind(SK_FOR_KEYWORD);
    else if (o("break"))    t.SetKind(SK_BREAK_KEYWORD);
    else if (o("continue")) t.SetKind(SK_CONTINUE_KEYWORD);
    else if (o("return"))   t.SetKind(SK_RETURN_KEYWORD);
    else {
        t.SetKind(SK_IDENTIFIER_TOKEN);
        t.SetName(name);
    }
#undef o
}

std::string Lexer::ReadSuffix() {
    std::string suffix{ };
    for (char c{ GetChar() }; IsLetter(c); c = GetChar()) {
        suffix += c;
        IncrementCursor();
    }
    return suffix;
}

void Lexer::SkipIntSuffixes(const SyntaxToken& t) {
    std::string suffix{ ReadSuffix() };
    std::string ciSuffix{ suffix };
    std::transform(ciSuffix.begin(), ciSuffix.end(), ciSuffix.begin(), std::toupper);

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

void Lexer::ReadFractionalLiteral(SyntaxToken& t) {
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
        t.SetKind(SK_FLOAT_CONSTANT_TOKEN);
        t.SetFloatValue(t.GetIntValue() + floatFrac);
    }
    else if (suffix.length() == 0) {
        t.SetKind(SK_DOUBLE_CONSTANT_TOKEN);
        t.SetDoubleValue(t.GetIntValue() + doubleFrac);
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
    }
}

void Lexer::ReadHexLiteral(SyntaxToken& t) {
    t.SetKind(SK_INT_CONSTANT_TOKEN);
    t.SetIntValue(0);

    while (IsHex(GetChar())) {
        t.SetIntValue(t.GetIntValue() * 16);

        if (GetChar() <= '9')
            t.SetIntValue(t.GetIntValue() + GetChar() - '0');
        else if (GetChar() <= 'F')
            t.SetIntValue(t.GetIntValue() + GetChar() - 'A' + 10);
        else
            t.SetIntValue(t.GetIntValue() + GetChar() - 'a' + 10);

        IncrementCursor();
    }

    SkipIntSuffixes(t);
}

void Lexer::ReadOctalLiteral(SyntaxToken& t) {
    t.SetKind(SK_INT_CONSTANT_TOKEN);
    t.SetIntValue(0);

    for (; IsDecimal(GetChar()); IncrementCursor()) {
        if (IsOctal(GetChar())) {
            t.SetIntValue((t.GetIntValue() * 8) - (GetChar() - '0'));
        }
        else {
            SOURCE_RANGE range{ };
            GetTokenRange(t, &range);

            LogAtRange(
                &range,
                LL_ERROR,
                "invalid digit \"%c\" in octal constant",
                GetChar()
            );
        }
    }

    SkipIntSuffixes(t);
}

void Lexer::ReadDecimalLiteral(SyntaxToken& t) {
    t.SetKind(SK_INT_CONSTANT_TOKEN);
    t.SetIntValue(0);

    for (; IsDecimal(GetChar()); IncrementCursor())
        t.SetIntValue((t.GetIntValue() * 10) + (GetChar() - '0'));

    if (GetChar() == '.') {
        IncrementCursor();
        ReadFractionalLiteral(t);
    }
    else {
        SkipIntSuffixes(t);
    }
}

void Lexer::ReadNumericalLiteral(SyntaxToken& t) {
    if (GetChar() == '0') {
        int wholeLength{ 0 };

        do {
            IncrementCursor();
        }
        while (GetChar() == '0');

        while (IsDecimal(Peek(wholeLength)))
            ++wholeLength;

        if (Peek(wholeLength) == '.') {
            ReadDecimalLiteral(t);
        }
        else if (GetChar() == 'X' || GetChar() == 'x') {
            IncrementCursor();
            ReadHexLiteral(t);
        }
        else {
            ReadOctalLiteral(t);
        }
    }
    else {
        ReadDecimalLiteral(t);
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
void Lexer::ReadCharLiteral(SyntaxToken& t) {
    SOURCE_RANGE range{ };

    IncrementCursor();
    t.SetKind(SK_INT_CONSTANT_TOKEN);

    if (GetChar() == '\n') {
        GetTokenRange(t, &range);
        LogAtRange(
            &range,
            LL_ERROR,
            "missing terminating ' character"
        );
    }
    else {
        t.SetIntValue(ReadCharEscapeSequence());

        if (GetChar() == '\'') {
            IncrementCursor();
        }
        else {
            while (GetChar() != '\'') {
                if (GetChar() == '\n') {
                    GetTokenRange(t, &range);
                    LogAtRange(
                        &range,
                        LL_ERROR,
                        "missing terminating ' character"
                    );
                    return;
                }

                IncrementCursor();
            }

            IncrementCursor();

            GetTokenRange(t, &range);
            LogAtRange(
                &range,
                LL_WARNING,
                "character constant too long for its type"
            );
        }
    }
}

/* Precondition: GetChar() == '"' */
void Lexer::ReadStringLiteral(SyntaxToken& t) {
    t.SetKind(SK_STRING_CONSTANT_TOKEN);
    IncrementCursor();

    t.SetStringValue("");

    while (GetChar() != '"') {
        if (GetChar() == '\n') {
            SOURCE_RANGE range;
            GetTokenRange(t, &range);

            LogAtRange(
                &range,
                LL_ERROR,
                "missing terminating \" character"
            );

            t.SetStringValue("");
            return;
        }

        std::string newValue{ t.GetStringValue() };
        newValue += ReadCharEscapeSequence();
        t.SetStringValue(newValue);
    }

    IncrementCursor();
}

SyntaxToken Lexer::ReadTokenOnce() {
    SyntaxToken result{ };

    while (IsWhitespace(GetChar()))
        IncrementCursor();

    if (l->CurrentFlags & ST_BEGINNING_OF_LINE)
        l->CurrentMode = LM_DEFAULT;

    result.SetFlags(l->CurrentFlags);
    SOURCE_RANGE temp{ result.GetLexemeRange() };
    temp.Location = l->CurrentLocation;
    result.SetLexemeRange(temp);

    if ((result.GetFlags() & ST_BEGINNING_OF_LINE) && GetChar() == '#') {
        IncrementCursor();
        result.SetKind(SK_HASH_TOKEN);
    }
    else {
        switch (GetChar()) {
        case 0: result.SetKind(SK_EOF_TOKEN); break;
        case '(': IncrementCursor(); result.SetKind(SK_LPAREN_TOKEN); break;
        case ')': IncrementCursor(); result.SetKind(SK_RPAREN_TOKEN); break;
        case '[': IncrementCursor(); result.SetKind(SK_LBRACKET_TOKEN); break;
        case ']': IncrementCursor(); result.SetKind(SK_RBRACKET_TOKEN); break;
        case '{': IncrementCursor(); result.SetKind(SK_LBRACE_TOKEN); break;
        case '}': IncrementCursor(); result.SetKind(SK_RBRACE_TOKEN); break;
        case ';': IncrementCursor(); result.SetKind(SK_SEMICOLON_TOKEN); break;
        case ',': IncrementCursor(); result.SetKind(SK_COMMA_TOKEN); break;
        case '~': IncrementCursor(); result.SetKind(SK_TILDE_TOKEN); break;
        case '?': IncrementCursor(); result.SetKind(SK_QUESTION_TOKEN); break;
        case ':': IncrementCursor(); result.SetKind(SK_COLON_TOKEN); break;

        case '.':
            IncrementCursor();
            if (IsDecimal(GetChar())) { result.SetIntValue(0); ReadFractionalLiteral(result); }
            else { result.SetKind(SK_DOT_TOKEN); }
            break;

        case '+':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result.SetKind(SK_PLUS_EQUALS_TOKEN); }
            else if (GetChar() == '+') { IncrementCursor(); result.SetKind(SK_PLUS_PLUS_TOKEN); }
            else { result.SetKind(SK_PLUS_TOKEN); }
            break;

        case '-':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result.SetKind(SK_MINUS_EQUALS_TOKEN); }
            else if (GetChar() == '-') { IncrementCursor(); result.SetKind(SK_MINUS_MINUS_TOKEN); }
            else if (GetChar() == '>') { IncrementCursor(); result.SetKind(SK_MINUS_GT_TOKEN); }
            else { result.SetKind(SK_MINUS_TOKEN); }
            break;

        case '*':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result.SetKind(SK_ASTERISK_EQUALS_TOKEN); }
            else { result.SetKind(SK_ASTERISK_TOKEN); }
            break;

        case '/':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result.SetKind(SK_SLASH_EQUALS_TOKEN);
            }
            else if (GetChar() == '*') {
                IncrementCursor();
                result.SetKind(SK_COMMENT_TOKEN);

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
                result.SetKind(SK_SLASH_TOKEN);
            }
            break;

        case '%':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result.SetKind(SK_PERCENT_EQUALS_TOKEN); }
            else { result.SetKind(SK_PERCENT_TOKEN); }
            break;

        case '<':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result.SetKind(SK_LT_EQUALS_TOKEN);
            }
            else if (GetChar() == '<') {
                IncrementCursor();
                if (GetChar() == '=') {
                    IncrementCursor();
                    result.SetKind(SK_LT_LT_EQUALS_TOKEN);
                }
                else {
                    result.SetKind(SK_LT_LT_TOKEN);
                }
            }
            else {
                result.SetKind(SK_LT_TOKEN);
            }
            break;

        case '>':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result.SetKind(SK_GT_EQUALS_TOKEN);
            }
            else if (GetChar() == '>') {
                IncrementCursor();
                if (GetChar() == '=') {
                    IncrementCursor();
                    result.SetKind(SK_GT_GT_EQUALS_TOKEN);
                }
                else {
                    result.SetKind(SK_GT_GT_TOKEN);
                }
            }
            else {
                result.SetKind(SK_GT_TOKEN);
            }
            break;

        case '=':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result.SetKind(SK_EQUALS_EQUALS_TOKEN); }
            else { result.SetKind(SK_EQUALS_TOKEN); }
            break;

        case '!':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result.SetKind(SK_EXCLAMATION_EQUALS_TOKEN); }
            else { result.SetKind(SK_EXCLAMATION_TOKEN); }
            break;

        case '&':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result.SetKind(SK_AMPERSAND_EQUALS_TOKEN); }
            else if (GetChar() == '&') { IncrementCursor(); result.SetKind(SK_AMPERSAND_AMPERSAND_TOKEN); }
            else { result.SetKind(SK_AMPERSAND_TOKEN); }
            break;

        case '^':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result.SetKind(SK_CARET_EQUALS_TOKEN); }
            else { result.SetKind(SK_CARET_TOKEN); } 
            break;

        case '|':
            IncrementCursor();
            if (GetChar() == '=') { IncrementCursor(); result.SetKind(SK_PIPE_EQUALS_TOKEN); }
            else if (GetChar() == '|') { IncrementCursor(); result.SetKind(SK_PIPE_PIPE_TOKEN); }
            else { result.SetKind(SK_PIPE_TOKEN); }
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
            ReadIdentifier(result);
            break;

        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            ReadNumericalLiteral(result);
            break;

        case '\'': ReadCharLiteral(result); break;
        case '"': ReadStringLiteral(result); break;

        default:
            result.SetKind(SK_STRAY_TOKEN);
            result.SetOffendingChar(GetChar());
            IncrementCursor();
            break;
        }
    }

    if (result.GetKind() == SK_COMMENT_TOKEN) {
        const int line{ result.GetLexemeRange().Location.Line };
        const int column{ result.GetLexemeRange().Location.Column };
        const char *base{ &l->Source->Lines[line][column] };
        // TODO: Re-implement length calculation.
        //result.LexemeRange.Length = static_cast<int>(l->Cursor - base);
        SOURCE_RANGE range{ result.GetLexemeRange() };
        range.Length = 1;
        result.SetLexemeRange(range);
    }
    else {
        const int end{ l->CurrentLocation.Column };
        const int start{ result.GetLexemeRange().Location.Column };
        SOURCE_RANGE range{ result.GetLexemeRange() };
        range.Length = end - start;
        result.SetLexemeRange(range);
    }

    if (l->CurrentMode == LM_DEFAULT && result.GetKind() == SK_HASH_TOKEN) {
        l->CurrentMode |= LM_PP_DIRECTIVE;
        l->CurrentMode |= LM_PP_DIRECTIVE_KW;
    }
    else if (l->CurrentMode & LM_PP_DIRECTIVE_KW) {
        l->CurrentMode &= ~LM_PP_DIRECTIVE_KW;

        if (result.GetKind() == SK_INCLUDE_DIRECTIVE_KEYWORD)
            l->CurrentMode |= LM_PP_ANGLED_STRING_CONSTANT;
    }
    else if (l->CurrentMode & LM_PP_ANGLED_STRING_CONSTANT) {
        l->CurrentMode &= ~LM_PP_ANGLED_STRING_CONSTANT;
    }

    return result;
}
