#define _CRT_SECURE_NO_WARNINGS
#include "lexer.hh"
#include "logger.hh"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LM_DEFAULT                   0
#define LM_PP_DIRECTIVE              1
#define LM_PP_DIRECTIVE_KW           2
#define LM_PP_ANGLED_STRING_CONSTANT 4

struct LEXER_IMPL {
    SourceFile*  Source;
    char*        Cursor;
    uint32_t     CurrentMode;
    int          CurrentFlags;
    SOURCE_LOC   CurrentLocation;
    SYNTAX_TOKEN CurrentToken;
};

Lexer::Lexer(IN SourceFile* input) :
    l{ new LEXER_IMPL{ } }
{
    l->Source                 = input;
    l->Cursor                 = l->Source->Contents;
    l->CurrentMode            = LM_DEFAULT;
    l->CurrentFlags           = ST_BEGINNING_OF_LINE;
    l->CurrentLocation.Source = l->Source;
}

Lexer::~Lexer() {
    delete l;
}

SYNTAX_TOKEN Lexer::ReadTokenDirect() {
    for (;;) {
        l->CurrentToken = ReadTokenOnce();

        if (l->CurrentToken.Base.Kind == SK_STRAY_TOKEN) {
            LogAtRange(
                &l->CurrentToken.Base.LexemeRange,
                LL_ERROR,
                "stray '%c' in program",
                l->CurrentToken.Value.OffendingChar
            );
        }
        else if (l->CurrentToken.Base.Kind != SK_COMMENT_TOKEN) {
            return l->CurrentToken;
        }
    }
}

inline bool IsWhitespace(char c) {
    return c == ' ' ||
           c == '\n' ||
           c == '\r' ||
           c == '\t';
}

inline bool IsLetter(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z');
}

inline bool IsDecimal(char c) {
    return c >= '0' && c <= '9';
}

inline bool IsOctal(char c) {
    return c >= '0' && c <= '7';
}

inline bool IsHex(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'A' && c <= 'F') ||
           (c >= 'a' && c <= 'f');
}

char Lexer::DecodeTrigraph(OUT int* charLength) {
    if (l->Cursor[0] == '?' && l->Cursor[1] == '?') {
        if (charLength)
            *charLength = 3;

        switch (l->Cursor[2]) {
            case '=':  return '#';
            case '(':  return '[';
            case '/':  return '\\';
            case ')':  return ']';
            case '\'': return '^';
            case '<':  return '{';
            case '!':  return '|';
            case '>':  return '}';
            case '-':  return '~';
        }
    }

    if (charLength)
        *charLength = 1;

    return *l->Cursor;
}

char Lexer::DecodeNewLineEscape(
    OUT  int    *charLength,
    OUT  int    *trailingWhitespaceLength
) {
    int firstCharLength;
    char firstChar{ DecodeTrigraph(&firstCharLength) };

    if (firstChar == '\\') {
        bool isOnlyWhitespace{ true };
        int lengthWithoutNewLine{ firstCharLength };

        while (l->Cursor[lengthWithoutNewLine] != '\n') {
            if (!IsWhitespace(l->Cursor[lengthWithoutNewLine])) {
                isOnlyWhitespace = false;
                break;
            }

            ++lengthWithoutNewLine;
        }

        if (isOnlyWhitespace) {
            int totalLength = lengthWithoutNewLine + 1;

            if (charLength)
                *charLength = totalLength;

            if (trailingWhitespaceLength) {
                int length{ lengthWithoutNewLine - firstCharLength };
                *trailingWhitespaceLength = length;
            }

            return ' ';
        }
    }

    if (charLength)
        *charLength = firstCharLength;
    if (trailingWhitespaceLength)
        *trailingWhitespaceLength = -1;

    return firstChar;
}

char Lexer::GetCharEx(OUT  int    *charLength) {
    return DecodeNewLineEscape(charLength, NULL);
}

char Lexer::GetChar() {
    return GetCharEx(NULL);
}

void Lexer::IncrementCursor() {
    int charLength;
    int trailingWhitespaceLength;
    char charValue{ DecodeNewLineEscape(
        &charLength, &trailingWhitespaceLength
    ) };

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
    IN   PSYNTAX_TOKEN t,
    OUT  PSOURCE_RANGE range
) {
    int line{ t->Base.LexemeRange.Location.Line };
    int column{ t->Base.LexemeRange.Location.Column };
    const char *base{ &l->Source->Lines[line][column] };

    range->Location = t->Base.LexemeRange.Location;
    range->Length = static_cast<int>(l->Cursor - base);
}

/* Precondition: GetChar() is [_A-Za-z] */
void Lexer::ReadIdentifier(OUT  PSYNTAX_TOKEN t) {
    SourceFile* source{ t->Base.LexemeRange.Location.Source };

    int line{ t->Base.LexemeRange.Location.Line };
    int column{ t->Base.LexemeRange.Location.Column };
    int length;

    const char *start{ &source->Lines[line][column] };

    for (;;) {
        char character{ GetChar() };

        if ((character == '_') ||
            (character == '$') ||
            (character >= 'A' && character <= 'Z') ||
            (character >= 'a' && character <= 'z') ||
            (character >= '0' && character <= '9'))
        {
            IncrementCursor();
        }
        else {
            break;
        }
    }

    length = static_cast<int>(l->Cursor - start) / sizeof(char);

#define o(kw) (length == (sizeof(kw) / sizeof(char) - 1) && !strncmp(start, kw, length))
    if (l->CurrentMode & LM_PP_DIRECTIVE_KW) {
             if (o("if"))       t->Base.Kind = SK_IF_DIRECTIVE_KEYWORD;
        else if (o("ifdef"))    t->Base.Kind = SK_IFDEF_DIRECTIVE_KEYWORD;
        else if (o("ifndef"))   t->Base.Kind = SK_IFNDEF_DIRECTIVE_KEYWORD;
        else if (o("elif"))     t->Base.Kind = SK_ELIF_DIRECTIVE_KEYWORD;
        else if (o("endif"))    t->Base.Kind = SK_ENDIF_DIRECTIVE_KEYWORD;
        else if (o("include"))  t->Base.Kind = SK_INCLUDE_DIRECTIVE_KEYWORD;
        else if (o("define"))   t->Base.Kind = SK_DEFINE_DIRECTIVE_KEYWORD;
        else if (o("undef"))    t->Base.Kind = SK_UNDEF_DIRECTIVE_KEYWORD;
        else if (o("line"))     t->Base.Kind = SK_LINE_DIRECTIVE_KEYWORD;
        else if (o("error"))    t->Base.Kind = SK_ERROR_DIRECTIVE_KEYWORD;
        else if (o("warning"))  t->Base.Kind = SK_WARNING_DIRECTIVE_KEYWORD;
    }
    else if (o("const"))    t->Base.Kind = SK_CONST_KEYWORD;
    else if (o("extern"))   t->Base.Kind = SK_EXTERN_KEYWORD;
    else if (o("static"))   t->Base.Kind = SK_STATIC_KEYWORD;
    else if (o("auto"))     t->Base.Kind = SK_AUTO_KEYWORD;
    else if (o("volatile")) t->Base.Kind = SK_VOLATILE_KEYWORD;
    else if (o("unsigned")) t->Base.Kind = SK_UNSIGNED_KEYWORD;
    else if (o("signed"))   t->Base.Kind = SK_SIGNED_KEYWORD;
    else if (o("void"))     t->Base.Kind = SK_VOID_KEYWORD;
    else if (o("char"))     t->Base.Kind = SK_CHAR_KEYWORD;
    else if (o("short"))    t->Base.Kind = SK_SHORT_KEYWORD;
    else if (o("int"))      t->Base.Kind = SK_INT_KEYWORD;
    else if (o("long"))     t->Base.Kind = SK_LONG_KEYWORD;
    else if (o("float"))    t->Base.Kind = SK_FLOAT_KEYWORD;
    else if (o("double"))   t->Base.Kind = SK_DOUBLE_KEYWORD;
    else if (o("enum"))     t->Base.Kind = SK_ENUM_KEYWORD;
    else if (o("struct"))   t->Base.Kind = SK_STRUCT_KEYWORD;
    else if (o("union"))    t->Base.Kind = SK_UNION_KEYWORD;
    else if (o("typedef"))  t->Base.Kind = SK_TYPEDEF_KEYWORD;
    else if (o("sizeof"))   t->Base.Kind = SK_SIZEOF_KEYWORD;
    else if (o("register")) t->Base.Kind = SK_REGISTER_KEYWORD;
    else if (o("goto"))     t->Base.Kind = SK_GOTO_KEYWORD;
    else if (o("if"))       t->Base.Kind = SK_IF_KEYWORD;
    else if (o("else"))     t->Base.Kind = SK_ELSE_KEYWORD;
    else if (o("switch"))   t->Base.Kind = SK_SWITCH_KEYWORD;
    else if (o("case"))     t->Base.Kind = SK_CASE_KEYWORD;
    else if (o("default"))  t->Base.Kind = SK_DEFAULT_KEYWORD;
    else if (o("do"))       t->Base.Kind = SK_DO_KEYWORD;
    else if (o("while"))    t->Base.Kind = SK_WHILE_KEYWORD;
    else if (o("for"))      t->Base.Kind = SK_FOR_KEYWORD;
    else if (o("break"))    t->Base.Kind = SK_BREAK_KEYWORD;
    else if (o("continue")) t->Base.Kind = SK_CONTINUE_KEYWORD;
    else if (o("return"))   t->Base.Kind = SK_RETURN_KEYWORD;
    else {
        t->Base.Kind = SK_IDENTIFIER_TOKEN;
        char* temp{ new char[length + 1]{ } };
        strncpy(temp, start, length);
        temp[length] = 0;
        t->Value.IdentifierName = temp;
        delete[] temp;
    }
#undef o
}

void Lexer::ReadSuffix(
    OUT  char   **suffix,
    OUT  int     *length
) {
    *length = 0;
    while (IsLetter(l->Cursor[*length]))
        ++*length;

    *suffix = new char[*length + 1]{ };
    for (int i{ 0 }; i < *length; ++i)
        (*suffix)[i] = l->Cursor[i];
    (*suffix)[*length] = 0;
    IncrementCursorBy(*length);
}

int Lexer::SkipUnsignedSuffix(IN_OUT char **cursor) {
    if (**cursor == 'U' || **cursor == 'u') {
        ++*cursor;
        return 1;
    }
    return 0;
}

int Lexer::SkipLongSuffix(IN_OUT char **cursor) {
    if (**cursor == 'L' || **cursor == 'l') {
        ++*cursor;
        if (**cursor == 'L' || **cursor == 'l')
            ++*cursor;
        return 1;
    }
    return 0;
}

void Lexer::SkipIntSuffixes(IN   PSYNTAX_TOKEN t) {
    int suffixLength{ 0 };
    char *suffix;
    char *suffixCursor;

    ReadSuffix(&suffix, &suffixLength);
    suffixCursor = suffix;

    if (SkipUnsignedSuffix(&suffixCursor))
        SkipLongSuffix(&suffixCursor);
    else if (SkipLongSuffix(&suffixCursor))
        SkipUnsignedSuffix(&suffixCursor);

    if (IsLetter(*suffixCursor)) {
        SOURCE_RANGE range;
        GetTokenRange(t, &range);

        LogAtRange(
            &range,
            LL_ERROR,
            "invalid suffix \"%s\" on integer constant",
            suffix
        );
    }

    delete[] suffix;
}

void Lexer::ReadFractionalLiteral(IN   PSYNTAX_TOKEN t) {
    float floatFrac{ 0.0F };
    float floatExp{ 0.1F };
    float doubleFrac{ 0.0 };
    float doubleExp{ 0.1 };
    char *suffix;
    int suffixLength;

    for (; IsDecimal(GetChar()); IncrementCursor()) {
        floatFrac += floatExp * (GetChar() - '0');
        doubleFrac += doubleExp * (GetChar() - '0');
        floatExp *= 0.1F;
        doubleExp *= 0.1;
    }

    ReadSuffix(&suffix, &suffixLength);

    if (*suffix == 'f' || *suffix == 'F') {
        t->Base.Kind = SK_FLOAT_CONSTANT_TOKEN;
        t->Value.FloatValue = t->Value.IntValue + floatFrac;
    }
    else {
        t->Base.Kind = SK_DOUBLE_CONSTANT_TOKEN;
        t->Value.DoubleValue = t->Value.IntValue + doubleFrac;
    }

    if (suffixLength > 1) {
        SOURCE_RANGE range;
        GetTokenRange(t, &range);

        LogAtRange(
            &range,
            LL_ERROR,
            "invalid suffix \"%s\" on floating constant",
            suffix
        );
    }

    delete[] suffix;
}

void Lexer::ReadHexLiteral(OUT  PSYNTAX_TOKEN t) {
    t->Base.Kind = SK_INT_CONSTANT_TOKEN;
    t->Value.IntValue = 0;

    while (IsHex(GetChar())) {
        t->Value.IntValue *= 16;

        if (GetChar() <= '9')
            t->Value.IntValue += GetChar() - '0';
        else if (GetChar() <= 'F')
            t->Value.IntValue += GetChar() - 'A' + 10;
        else
            t->Value.IntValue += GetChar() - 'a' + 10;

        IncrementCursor();
    }

    SkipIntSuffixes(t);
}

void Lexer::ReadOctalLiteral(OUT  PSYNTAX_TOKEN t) {
    t->Base.Kind = SK_INT_CONSTANT_TOKEN;
    t->Value.IntValue = 0;

    for (; IsDecimal(GetChar()); IncrementCursor()) {
        if (IsOctal(GetChar())) {
            t->Value.IntValue = (t->Value.IntValue * 8) - (GetChar() - '0');
        }
        else {
            SOURCE_RANGE range;
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

void Lexer::ReadDecimalLiteral(OUT  PSYNTAX_TOKEN t) {
    t->Base.Kind = SK_INT_CONSTANT_TOKEN;
    t->Value.IntValue = 0;

    for (; IsDecimal(GetChar()); IncrementCursor())
        t->Value.IntValue = (t->Value.IntValue * 10) + (GetChar() - '0');

    if (GetChar() == '.') {
        IncrementCursor();
        ReadFractionalLiteral(t);
    }
    else {
        SkipIntSuffixes(t);
    }
}

void Lexer::ReadNumericalLiteral(OUT  PSYNTAX_TOKEN t) {
    if (GetChar() == '0') {
        int wholeLength{ 0 };

        do {
            IncrementCursor();
        }
        while (GetChar() == '0');

        while (IsDecimal(l->Cursor[wholeLength]))
            ++wholeLength;

        if (l->Cursor[wholeLength] == '.') {
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
    int result;

    if (GetChar() == '\\') {
        SOURCE_RANGE errorRange;
        int digitCount;

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
void Lexer::ReadCharLiteral(OUT  PSYNTAX_TOKEN t) {
    SOURCE_RANGE range;

    IncrementCursor();
    t->Base.Kind = SK_INT_CONSTANT_TOKEN;

    if (GetChar() == '\n') {
        GetTokenRange(t, &range);
        LogAtRange(
            &range,
            LL_ERROR,
            "missing terminating ' character"
        );
    }
    else {
        t->Value.IntValue = ReadCharEscapeSequence();

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
void Lexer::ReadStringLiteral(OUT  PSYNTAX_TOKEN t) {
    t->Base.Kind = SK_STRING_CONSTANT_TOKEN;
    IncrementCursor();

    t->Value.StringValue.clear();

    while (GetChar() != '"') {
        if (GetChar() == '\n') {
            SOURCE_RANGE range;
            GetTokenRange(t, &range);

            LogAtRange(
                &range,
                LL_ERROR,
                "missing terminating \" character"
            );

            t->Value.StringValue = "";
            return;
        }

        t->Value.StringValue += ReadCharEscapeSequence();
    }

    IncrementCursor();
}

SYNTAX_TOKEN Lexer::ReadTokenOnce() {
    SYNTAX_TOKEN result{ };

    while (IsWhitespace(GetChar()))
        IncrementCursor();

    if (l->CurrentFlags & ST_BEGINNING_OF_LINE)
        l->CurrentMode = LM_DEFAULT;

    result.Flags = l->CurrentFlags;
    result.Base.LexemeRange.Location = l->CurrentLocation;

    if ((result.Flags & ST_BEGINNING_OF_LINE) && GetChar() == '#') {
        IncrementCursor();
        result.Base.Kind = SK_HASH_TOKEN;
    }
    else {
        switch (GetChar()) {
        case 0:
            result.Base.Kind = SK_EOF_TOKEN;
            break;

        case '(':
            IncrementCursor();
            result.Base.Kind = SK_LPAREN_TOKEN;
            break;

        case ')':
            IncrementCursor();
            result.Base.Kind = SK_RPAREN_TOKEN;
            break;

        case '[':
            IncrementCursor();
            result.Base.Kind = SK_LBRACKET_TOKEN;
            break;

        case ']':
            IncrementCursor();
            result.Base.Kind = SK_RBRACKET_TOKEN;
            break;

        case '{':
            IncrementCursor();
            result.Base.Kind = SK_LBRACE_TOKEN;
            break;

        case '}':
            IncrementCursor();
            result.Base.Kind = SK_RBRACE_TOKEN;
            break;

        case ';':
            IncrementCursor();
            result.Base.Kind = SK_SEMICOLON_TOKEN;
            break;

        case ',':
            IncrementCursor();
            result.Base.Kind = SK_COMMA_TOKEN;
            break;

        case '~':
            IncrementCursor();
            result.Base.Kind = SK_TILDE_TOKEN;
            break;

        case '?':
            IncrementCursor();
            result.Base.Kind = SK_QUESTION_TOKEN;
            break;

        case ':':
            IncrementCursor();
            result.Base.Kind = SK_COLON_TOKEN;
            break;

        case '.':
            IncrementCursor();
            if (IsDecimal(GetChar())) {
                result.Value.IntValue = 0;
                ReadFractionalLiteral(&result);
            }
            else {
                result.Base.Kind = SK_DOT_TOKEN;
            }
            break;

        case '+':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result.Base.Kind = SK_PLUS_EQUALS_TOKEN;
            }
            else if (GetChar() == '+') {
                IncrementCursor();
                result.Base.Kind = SK_PLUS_PLUS_TOKEN;
            }
            else {
                result.Base.Kind = SK_PLUS_TOKEN;
            }
            break;

        case '-':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result.Base.Kind = SK_MINUS_EQUALS_TOKEN;
            }
            else if (GetChar() == '-') {
                IncrementCursor();
                result.Base.Kind = SK_MINUS_MINUS_TOKEN;
            }
            else if (GetChar() == '>') {
                IncrementCursor();
                result.Base.Kind = SK_MINUS_GT_TOKEN;
            }
            else {
                result.Base.Kind = SK_MINUS_TOKEN;
            }
            break;

        case '*':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result.Base.Kind = SK_ASTERISK_EQUALS_TOKEN;
            }
            else {
                result.Base.Kind = SK_ASTERISK_TOKEN;
            }
            break;

        case '/':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result.Base.Kind = SK_SLASH_EQUALS_TOKEN;
            }
            else if (GetChar() == '*') {
                IncrementCursor();
                result.Base.Kind = SK_COMMENT_TOKEN;

                for (;;) {
                    SOURCE_RANGE range;

                    if (GetChar() == '*') {
                        IncrementCursor();

                        if (GetChar() == '/') {
                            IncrementCursor();
                            break;
                        }
                        else if (GetChar() == 0) {
                            GetTokenRange(&result, &range);
                            LogAtRange(
                                &range,
                                LL_ERROR,
                                "unterminated comment"
                            );
                            break;
                        }
                    }
                    else if (GetChar() == 0) {
                        GetTokenRange(&result, &range);
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
                result.Base.Kind = SK_SLASH_TOKEN;
            }
            break;

        case '%':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result.Base.Kind = SK_PERCENT_EQUALS_TOKEN;
            }
            else {
                result.Base.Kind = SK_PERCENT_TOKEN;
            }
            break;

        case '<':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result.Base.Kind = SK_LT_EQUALS_TOKEN;
            }
            else if (GetChar() == '<') {
                IncrementCursor();
                if (GetChar() == '=') {
                    IncrementCursor();
                    result.Base.Kind = SK_LT_LT_EQUALS_TOKEN;
                }
                else {
                    result.Base.Kind = SK_LT_LT_TOKEN;
                }
            }
            else {
                result.Base.Kind = SK_LT_TOKEN;
            }
            break;

        case '>':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result.Base.Kind = SK_GT_EQUALS_TOKEN;
            }
            else if (GetChar() == '>') {
                IncrementCursor();
                if (GetChar() == '=') {
                    IncrementCursor();
                    result.Base.Kind = SK_GT_GT_EQUALS_TOKEN;
                }
                else {
                    result.Base.Kind = SK_GT_GT_TOKEN;
                }
            }
            else {
                result.Base.Kind = SK_GT_TOKEN;
            }
            break;

        case '=':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result.Base.Kind = SK_EQUALS_EQUALS_TOKEN;
            }
            else {
                result.Base.Kind = SK_EQUALS_TOKEN;
            }
            break;

        case '!':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result.Base.Kind = SK_EXCLAMATION_EQUALS_TOKEN;
            }
            else {
                result.Base.Kind = SK_EXCLAMATION_TOKEN;
            }
            break;

        case '&':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result.Base.Kind = SK_AMPERSAND_EQUALS_TOKEN;
            }
            else if (GetChar() == '&') {
                IncrementCursor();
                result.Base.Kind = SK_AMPERSAND_AMPERSAND_TOKEN;
            }
            else {
                result.Base.Kind = SK_AMPERSAND_TOKEN;
            }
            break;

        case '^':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result.Base.Kind = SK_CARET_EQUALS_TOKEN;
            }
            else {
                result.Base.Kind = SK_CARET_TOKEN;
            } 
            break;

        case '|':
            IncrementCursor();
            if (GetChar() == '=') {
                IncrementCursor();
                result.Base.Kind = SK_PIPE_EQUALS_TOKEN;
            }
            else if (GetChar() == '|') {
                IncrementCursor();
                result.Base.Kind = SK_PIPE_PIPE_TOKEN;
            }
            else {
                result.Base.Kind = SK_PIPE_TOKEN;
            }
            break;

        case '_': case '$':
        case 'A': case 'B': case 'C': case 'D':
        case 'E': case 'F': case 'G': case 'H':
        case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P':
        case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z': 
        case 'a': case 'b': case 'c': case 'd':
        case 'e': case 'f': case 'g': case 'h':
        case 'i': case 'j': case 'k': case 'l':
        case 'm': case 'n': case 'o': case 'p':
        case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x':
        case 'y': case 'z': 
            ReadIdentifier(&result);
            break;

        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        case '8': case '9':
            ReadNumericalLiteral(&result);
            break;

        case '\'':
            ReadCharLiteral(&result);
            break;

        case '"':
            ReadStringLiteral(&result);
            break;

        default:
            result.Base.Kind = SK_STRAY_TOKEN;
            result.Value.OffendingChar = GetChar();
            IncrementCursor();
            break;
        }
    }

    if (result.Base.Kind == SK_COMMENT_TOKEN) {
        int line{ result.Base.LexemeRange.Location.Line };
        int column{ result.Base.LexemeRange.Location.Column };
        const char *base{ &l->Source->Lines[line][column] };
        result.Base.LexemeRange.Length = static_cast<int>(l->Cursor - base);
    }
    else {
        int end{ l->CurrentLocation.Column };
        int start{ result.Base.LexemeRange.Location.Column };
        result.Base.LexemeRange.Length = end - start;
    }

    if (l->CurrentMode == LM_DEFAULT && result.Base.Kind == SK_HASH_TOKEN) {
        l->CurrentMode |= LM_PP_DIRECTIVE;
        l->CurrentMode |= LM_PP_DIRECTIVE_KW;
    }
    else if (l->CurrentMode & LM_PP_DIRECTIVE_KW) {
        l->CurrentMode &= ~LM_PP_DIRECTIVE_KW;

        if (result.Base.Kind == SK_INCLUDE_DIRECTIVE_KEYWORD)
            l->CurrentMode |= LM_PP_ANGLED_STRING_CONSTANT;
    }
    else if (l->CurrentMode & LM_PP_ANGLED_STRING_CONSTANT) {
        l->CurrentMode &= ~LM_PP_ANGLED_STRING_CONSTANT;
    }

    return result;
}
