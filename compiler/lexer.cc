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

struct tagLexer {
    PSOURCE_FILE  Source;
    char         *Cursor;
    uint32_t      CurrentMode;
    int           CurrentFlags;
    SOURCE_LOC    CurrentLocation;
    SYNTAX_TOKEN  CurrentToken;
};

PLEXER CreateLexer(
    IN PSOURCE_FILE input
)
{
    PLEXER lexer                  = new Lexer;
    lexer->Source                 = input;
    lexer->Cursor                 = lexer->Source->Contents;
    lexer->CurrentMode            = LM_DEFAULT;
    lexer->CurrentFlags           = ST_BEGINNING_OF_LINE;
    lexer->CurrentLocation.Source = lexer->Source;
    return lexer;
}

void DeleteLexer(
    THIS PLEXER l
)
{
    DeleteSyntaxNode((PSYNTAX_NODE) &l->CurrentToken);
    delete l;
}

static SYNTAX_TOKEN ReadTokenOnce(THIS PLEXER l);

PSYNTAX_TOKEN ReadTokenDirect(
    THIS PLEXER l
)
{
    for (;;) {
        DeleteSyntaxNode((PSYNTAX_NODE) &l->CurrentToken);

        l->CurrentToken = ReadTokenOnce(l);

        if (l->CurrentToken.Base.Kind == SK_STRAY_TOKEN) {
            LogAtRange(
                &l->CurrentToken.Base.LexemeRange,
                LL_ERROR,
                "stray '%c' in program",
                l->CurrentToken.Value.OffendingChar
            );
        }
        else if (l->CurrentToken.Base.Kind != SK_COMMENT_TOKEN) {
            return &l->CurrentToken;
        }
    }
}

#define IsWhitespace(c) ((c) == ' ' || \
                         (c) == '\n' || \
                         (c) == '\r' || \
                         (c) == '\t')

#define IsLetter(c) (((c) >= 'A' && (c) <= 'Z') || \
                     ((c) >= 'a' && (c) <= 'z'))

#define IsDecimal(c) ((c) >= '0' && (c) <= '9')

#define IsOctal(c) ((c) >= '0' && (c) <= '7')

#define IsHex(c) (((c) >= '0' && (c) <= '9') || \
                  ((c) >= 'A' && (c) <= 'F') || \
                  ((c) >= 'a' && (c) <= 'f'))

static char DecodeTrigraph(
    THIS PLEXER  l,
    OUT  int    *charLength
)
{
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

static char DecodeNewLineEscape(
    THIS PLEXER  l,
    OUT  int    *charLength,
    OUT  int    *trailingWhitespaceLength
)
{
    int firstCharLength;
    char firstChar = DecodeTrigraph(l, &firstCharLength);

    if (firstChar == '\\') {
        bool isOnlyWhitespace = true;
        int lengthWithoutNewLine = firstCharLength;

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
                int length = lengthWithoutNewLine - firstCharLength;
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

static char GetCharEx(
    THIS PLEXER  l,
    OUT  int    *charLength
)
{
    return DecodeNewLineEscape(l, charLength, NULL);
}

static char GetChar(THIS PLEXER l) {
    return GetCharEx(l, NULL);
}

static void IncrementCursor(THIS PLEXER l) {
    int charLength;
    int trailingWhitespaceLength;
    char charValue = DecodeNewLineEscape(
        l, &charLength, &trailingWhitespaceLength
    );

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

static void IncrementCursorBy(
    THIS PLEXER l,
    IN   int    amount
)
{
    while (amount--)
        IncrementCursor(l);
}

static void GetTokenRange(
    THIS PLEXER        l,
    IN   PSYNTAX_TOKEN t,
    OUT  PSOURCE_RANGE range
)
{
    int line = t->Base.LexemeRange.Location.Line;
    int column = t->Base.LexemeRange.Location.Column;
    const char *base = &l->Source->Lines[line][column];

    range->Location = t->Base.LexemeRange.Location;
    range->Length = (int) (l->Cursor - base);
}

/* Precondition: GetChar(l) is [_A-Za-z] */
static void ReadIdentifier(
    THIS PLEXER        l,
    OUT  PSYNTAX_TOKEN t
)
{
    PSOURCE_FILE source = t->Base.LexemeRange.Location.Source;

    int line   = t->Base.LexemeRange.Location.Line,
        column = t->Base.LexemeRange.Location.Column,
        length;

    const char *start = &source->Lines[line][column];

    for (;;) {
        char character = GetChar(l);

        if ((character == '_') ||
            (character == '$') ||
            (character >= 'A' && character <= 'Z') ||
            (character >= 'a' && character <= 'z') ||
            (character >= '0' && character <= '9'))
        {
            IncrementCursor(l);
        }
        else {
            break;
        }
    }

    length = (int) (l->Cursor - start) / sizeof(char);

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
        t->Value.IdentifierName = new char[length + 1];
        strncpy(t->Value.IdentifierName, start, length);
        t->Value.IdentifierName[length] = 0;
    }
#undef o
}

static void ReadSuffix(
    THIS PLEXER   l,
    OUT  char   **suffix,
    OUT  int     *length
)
{
    int i;

    *length = 0;
    while (IsLetter(l->Cursor[*length]))
        ++*length;

    *suffix = new char[*length + 1];
    for (i = 0; i < *length; ++i)
        (*suffix)[i] = l->Cursor[i];
    (*suffix)[*length] = 0;
    IncrementCursorBy(l, *length);
}

static int SkipUnsignedSuffix(IN_OUT char **cursor) {
    if (**cursor == 'U' || **cursor == 'u') {
        ++*cursor;
        return 1;
    }
    return 0;
}

static int SkipLongSuffix(IN_OUT char **cursor) {
    if (**cursor == 'L' || **cursor == 'l') {
        ++*cursor;
        if (**cursor == 'L' || **cursor == 'l')
            ++*cursor;
        return 1;
    }
    return 0;
}

static void SkipIntSuffixes(
    THIS PLEXER        l,
    IN   PSYNTAX_TOKEN t
)
{
    int suffixLength = 0;
    char *suffix;
    char *suffixCursor;

    ReadSuffix(l, &suffix, &suffixLength);
    suffixCursor = suffix;

    if (SkipUnsignedSuffix(&suffixCursor))
        SkipLongSuffix(&suffixCursor);
    else if (SkipLongSuffix(&suffixCursor))
        SkipUnsignedSuffix(&suffixCursor);

    if (IsLetter(*suffixCursor)) {
        SOURCE_RANGE range;
        GetTokenRange(l, t, &range);

        LogAtRange(
            &range,
            LL_ERROR,
            "invalid suffix \"%s\" on integer constant",
            suffix
        );
    }

    delete[] suffix;
}

static void ReadFractionalLiteral(
    THIS PLEXER        l,
    IN   PSYNTAX_TOKEN t
)
{
    float floatFrac = 0.0F;
    float floatExp = 0.1F;
    float doubleFrac = 0.0;
    float doubleExp = 0.1;
    char *suffix;
    int suffixLength;

    for (; IsDecimal(GetChar(l)); IncrementCursor(l)) {
        floatFrac += floatExp * (GetChar(l) - '0');
        doubleFrac += doubleExp * (GetChar(l) - '0');
        floatExp *= 0.1F;
        doubleExp *= 0.1;
    }

    ReadSuffix(l, &suffix, &suffixLength);

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
        GetTokenRange(l, t, &range);

        LogAtRange(
            &range,
            LL_ERROR,
            "invalid suffix \"%s\" on floating constant",
            suffix
        );
    }

    delete[] suffix;
}

static void ReadHexLiteral(
    THIS PLEXER        l,
    OUT  PSYNTAX_TOKEN t
)
{
    t->Base.Kind = SK_INT_CONSTANT_TOKEN;
    t->Value.IntValue = 0;

    while (IsHex(GetChar(l))) {
        t->Value.IntValue *= 16;

        if (GetChar(l) <= '9')
            t->Value.IntValue += GetChar(l) - '0';
        else if (GetChar(l) <= 'F')
            t->Value.IntValue += GetChar(l) - 'A' + 10;
        else
            t->Value.IntValue += GetChar(l) - 'a' + 10;

        IncrementCursor(l);
    }

    SkipIntSuffixes(l, t);
}

static void ReadOctalLiteral(
    THIS PLEXER        l,
    OUT  PSYNTAX_TOKEN t
)
{
    t->Base.Kind = SK_INT_CONSTANT_TOKEN;
    t->Value.IntValue = 0;

    for (; IsDecimal(GetChar(l)); IncrementCursor(l)) {
        if (IsOctal(GetChar(l))) {
            t->Value.IntValue = (t->Value.IntValue * 8) - (GetChar(l) - '0');
        }
        else {
            SOURCE_RANGE range;
            GetTokenRange(l, t, &range);

            LogAtRange(
                &range,
                LL_ERROR,
                "invalid digit \"%c\" in octal constant",
                GetChar(l)
            );
        }
    }

    SkipIntSuffixes(l, t);
}

static void ReadDecimalLiteral(
    THIS PLEXER        l,
    OUT  PSYNTAX_TOKEN t
)
{
    t->Base.Kind = SK_INT_CONSTANT_TOKEN;
    t->Value.IntValue = 0;

    for (; IsDecimal(GetChar(l)); IncrementCursor(l))
        t->Value.IntValue = (t->Value.IntValue * 10) + (GetChar(l) - '0');

    if (GetChar(l) == '.') {
        IncrementCursor(l);
        ReadFractionalLiteral(l, t);
    }
    else {
        SkipIntSuffixes(l, t);
    }
}

static void ReadNumericalLiteral(
    THIS PLEXER        l,
    OUT  PSYNTAX_TOKEN t
)
{
    if (GetChar(l) == '0') {
        int wholeLength = 0;

        do {
            IncrementCursor(l);
        }
        while (GetChar(l) == '0');

        while (IsDecimal(l->Cursor[wholeLength]))
            ++wholeLength;

        if (l->Cursor[wholeLength] == '.') {
            ReadDecimalLiteral(l, t);
        }
        else if (GetChar(l) == 'X' || GetChar(l) == 'x') {
            IncrementCursor(l);
            ReadHexLiteral(l, t);
        }
        else {
            ReadOctalLiteral(l, t);
        }
    }
    else {
        ReadDecimalLiteral(l, t);
    }
}

static int ReadCharEscapeSequence(THIS PLEXER l) {
    int result;

    if (GetChar(l) == '\\') {
        SOURCE_RANGE errorRange;
        int digitCount;

        errorRange.Location = l->CurrentLocation;
        errorRange.Length = 2;

        IncrementCursor(l);

        switch (GetChar(l)) {
            case '\'': IncrementCursor(l); result = '\''; break;
            case '"':  IncrementCursor(l); result = '\"'; break;
            case '?':  IncrementCursor(l); result = '\?'; break;
            case '\\': IncrementCursor(l); result = '\\'; break;
            case 'a':  IncrementCursor(l); result = '\a'; break;
            case 'b':  IncrementCursor(l); result = '\b'; break;
            case 'f':  IncrementCursor(l); result = '\f'; break;
            case 'n':  IncrementCursor(l); result = '\n'; break;
            case 'r':  IncrementCursor(l); result = '\r'; break;
            case 't':  IncrementCursor(l); result = '\t'; break;
            case 'v':  IncrementCursor(l); result = '\v'; break;

            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7':
                result = 0;
                digitCount = 0;

                while (digitCount < 3 && IsOctal(GetChar(l))) {
                    result = (result * 8) + (GetChar(l) - '0');
                    ++digitCount;
                    IncrementCursor(l);
                }

                break;

            case 'x':
                IncrementCursor(l);
                result = 0;

                for (; IsHex(GetChar(l)); IncrementCursor(l)) {
                    result *= 16;
                    if (GetChar(l) <= '9')
                        result += GetChar(l) - '0';
                    else if (GetChar(l) <= 'F')
                        result += GetChar(l) - 'A' + 10;
                    else if (GetChar(l) <= 'f')
                        result += GetChar(l) - 'a' + 10;
                }

                break;

            default:
                LogAtRange(
                    &errorRange,
                    LL_ERROR,
                    "unknown escape sequence: '\\%c'",
                    GetChar(l)
                );

                IncrementCursor(l);
                break;
        }
    }
    else {
        result = GetChar(l);
        IncrementCursor(l);
    }

    return result;
}

/* Precondition: GetChar(l) == '\'' */
static void ReadCharLiteral(
    THIS PLEXER        l,
    OUT  PSYNTAX_TOKEN t
)
{
    SOURCE_RANGE range;

    IncrementCursor(l);
    t->Base.Kind = SK_INT_CONSTANT_TOKEN;

    if (GetChar(l) == '\n') {
        GetTokenRange(l, t, &range);
        LogAtRange(
            &range,
            LL_ERROR,
            "missing terminating ' character"
        );
    }
    else {
        t->Value.IntValue = ReadCharEscapeSequence(l);

        if (GetChar(l) == '\'') {
            IncrementCursor(l);
        }
        else {
            while (GetChar(l) != '\'') {
                if (GetChar(l) == '\n') {
                    GetTokenRange(l, t, &range);
                    LogAtRange(
                        &range,
                        LL_ERROR,
                        "missing terminating ' character"
                    );
                    return;
                }

                IncrementCursor(l);
            }

            IncrementCursor(l);

            GetTokenRange(l, t, &range);
            LogAtRange(
                &range,
                LL_WARNING,
                "character constant too long for its type"
            );
        }
    }
}

/* Precondition: GetChar(l) == '"' */
static void ReadStringLiteral(
    THIS PLEXER        l,
    OUT  PSYNTAX_TOKEN t
)
{
    int length = 0;
    int capacity = 12;

    t->Base.Kind = SK_STRING_CONSTANT_TOKEN;
    IncrementCursor(l);

    t->Value.StringValue = new char[capacity + 1];
    t->Value.StringValue[0] = 0;

    while (GetChar(l) != '"') {
        if (GetChar(l) == '\n') {
            SOURCE_RANGE range;
            GetTokenRange(l, t, &range);

            LogAtRange(
                &range,
                LL_ERROR,
                "missing terminating \" character"
            );

            if (t->Value.StringValue != NULL)
                delete[] t->Value.StringValue;

            t->Value.StringValue = NULL;
            return;
        }

        if (length >= capacity) {
            capacity *= 2;
            t->Value.StringValue = static_cast<char*>(realloc(t->Value.StringValue, capacity + 1));
        }

        t->Value.StringValue[length] = ReadCharEscapeSequence(l);
        ++length;
    }

    IncrementCursor(l);
    t->Value.StringValue[length] = 0;
}

static SYNTAX_TOKEN ReadTokenOnce(THIS PLEXER l)
{
    SYNTAX_TOKEN result;
    memset(&result, 0, sizeof(result));

    while (IsWhitespace(GetChar(l)))
        IncrementCursor(l);

    if (l->CurrentFlags & ST_BEGINNING_OF_LINE)
        l->CurrentMode = LM_DEFAULT;

    result.Flags = l->CurrentFlags;
    result.Base.LexemeRange.Location = l->CurrentLocation;

    if ((result.Flags & ST_BEGINNING_OF_LINE) && GetChar(l) == '#') {
        IncrementCursor(l);
        result.Base.Kind = SK_HASH_TOKEN;
    }
    else {
        switch (GetChar(l)) {
        case 0:
            result.Base.Kind = SK_EOF_TOKEN;
            break;

        case '(':
            IncrementCursor(l);
            result.Base.Kind = SK_LPAREN_TOKEN;
            break;

        case ')':
            IncrementCursor(l);
            result.Base.Kind = SK_RPAREN_TOKEN;
            break;

        case '[':
            IncrementCursor(l);
            result.Base.Kind = SK_LBRACKET_TOKEN;
            break;

        case ']':
            IncrementCursor(l);
            result.Base.Kind = SK_RBRACKET_TOKEN;
            break;

        case '{':
            IncrementCursor(l);
            result.Base.Kind = SK_LBRACE_TOKEN;
            break;

        case '}':
            IncrementCursor(l);
            result.Base.Kind = SK_RBRACE_TOKEN;
            break;

        case ';':
            IncrementCursor(l);
            result.Base.Kind = SK_SEMICOLON_TOKEN;
            break;

        case ',':
            IncrementCursor(l);
            result.Base.Kind = SK_COMMA_TOKEN;
            break;

        case '~':
            IncrementCursor(l);
            result.Base.Kind = SK_TILDE_TOKEN;
            break;

        case '?':
            IncrementCursor(l);
            result.Base.Kind = SK_QUESTION_TOKEN;
            break;

        case ':':
            IncrementCursor(l);
            result.Base.Kind = SK_COLON_TOKEN;
            break;

        case '.':
            IncrementCursor(l);
            if (IsDecimal(GetChar(l))) {
                result.Value.IntValue = 0;
                ReadFractionalLiteral(l, &result);
            }
            else {
                result.Base.Kind = SK_DOT_TOKEN;
            }
            break;

        case '+':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Base.Kind = SK_PLUS_EQUALS_TOKEN;
            }
            else if (GetChar(l) == '+') {
                IncrementCursor(l);
                result.Base.Kind = SK_PLUS_PLUS_TOKEN;
            }
            else {
                result.Base.Kind = SK_PLUS_TOKEN;
            }
            break;

        case '-':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Base.Kind = SK_MINUS_EQUALS_TOKEN;
            }
            else if (GetChar(l) == '-') {
                IncrementCursor(l);
                result.Base.Kind = SK_MINUS_MINUS_TOKEN;
            }
            else if (GetChar(l) == '>') {
                IncrementCursor(l);
                result.Base.Kind = SK_MINUS_GT_TOKEN;
            }
            else {
                result.Base.Kind = SK_MINUS_TOKEN;
            }
            break;

        case '*':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Base.Kind = SK_ASTERISK_EQUALS_TOKEN;
            }
            else {
                result.Base.Kind = SK_ASTERISK_TOKEN;
            }
            break;

        case '/':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Base.Kind = SK_SLASH_EQUALS_TOKEN;
            }
            else if (GetChar(l) == '*') {
                IncrementCursor(l);
                result.Base.Kind = SK_COMMENT_TOKEN;

                for (;;) {
                    SOURCE_RANGE range;

                    if (GetChar(l) == '*') {
                        IncrementCursor(l);

                        if (GetChar(l) == '/') {
                            IncrementCursor(l);
                            break;
                        }
                        else if (GetChar(l) == 0) {
                            GetTokenRange(l, &result, &range);
                            LogAtRange(
                                &range,
                                LL_ERROR,
                                "unterminated comment"
                            );
                            break;
                        }
                    }
                    else if (GetChar(l) == 0) {
                        GetTokenRange(l, &result, &range);
                        LogAtRange(
                            &range,
                            LL_ERROR,
                            "unterminated comment"
                        );
                        break;
                    }
                    else {
                        IncrementCursor(l);
                    }
                }
            }
            else {
                result.Base.Kind = SK_SLASH_TOKEN;
            }
            break;

        case '%':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Base.Kind = SK_PERCENT_EQUALS_TOKEN;
            }
            else {
                result.Base.Kind = SK_PERCENT_TOKEN;
            }
            break;

        case '<':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Base.Kind = SK_LT_EQUALS_TOKEN;
            }
            else if (GetChar(l) == '<') {
                IncrementCursor(l);
                if (GetChar(l) == '=') {
                    IncrementCursor(l);
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
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Base.Kind = SK_GT_EQUALS_TOKEN;
            }
            else if (GetChar(l) == '>') {
                IncrementCursor(l);
                if (GetChar(l) == '=') {
                    IncrementCursor(l);
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
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Base.Kind = SK_EQUALS_EQUALS_TOKEN;
            }
            else {
                result.Base.Kind = SK_EQUALS_TOKEN;
            }
            break;

        case '!':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Base.Kind = SK_EXCLAMATION_EQUALS_TOKEN;
            }
            else {
                result.Base.Kind = SK_EXCLAMATION_TOKEN;
            }
            break;

        case '&':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Base.Kind = SK_AMPERSAND_EQUALS_TOKEN;
            }
            else if (GetChar(l) == '&') {
                IncrementCursor(l);
                result.Base.Kind = SK_AMPERSAND_AMPERSAND_TOKEN;
            }
            else {
                result.Base.Kind = SK_AMPERSAND_TOKEN;
            }
            break;

        case '^':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Base.Kind = SK_CARET_EQUALS_TOKEN;
            }
            else {
                result.Base.Kind = SK_CARET_TOKEN;
            } 
            break;

        case '|':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Base.Kind = SK_PIPE_EQUALS_TOKEN;
            }
            else if (GetChar(l) == '|') {
                IncrementCursor(l);
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
            ReadIdentifier(l, &result);
            break;

        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        case '8': case '9':
            ReadNumericalLiteral(l, &result);
            break;

        case '\'':
            ReadCharLiteral(l, &result);
            break;

        case '"':
            ReadStringLiteral(l, &result);
            break;

        default:
            result.Base.Kind = SK_STRAY_TOKEN;
            result.Value.OffendingChar = GetChar(l);
            IncrementCursor(l);
            break;
        }
    }

    if (result.Base.Kind == SK_COMMENT_TOKEN) {
        int line = result.Base.LexemeRange.Location.Line;
        int column = result.Base.LexemeRange.Location.Column;
        const char *base = &l->Source->Lines[line][column];
        result.Base.LexemeRange.Length = (int) (l->Cursor - base);
    }
    else {
        int end = l->CurrentLocation.Column;
        int start = result.Base.LexemeRange.Location.Column;
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
