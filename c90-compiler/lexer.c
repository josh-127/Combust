#include "lexer.h"
#include "error.h"
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
    TOKEN         CurrentToken;
};

PLEXER CreateLexer(
    IN PSOURCE_FILE input
)
{
    PLEXER lexer                  = calloc(1, sizeof(Lexer));
    lexer->Source                 = input;
    lexer->Cursor                 = lexer->Source->Contents;
    lexer->CurrentMode            = LM_DEFAULT;
    lexer->CurrentFlags           = TOKENFLAG_BOL;
    lexer->CurrentLocation.Line   = 0;
    lexer->CurrentLocation.Column = 0;
    lexer->CurrentLocation.Source = lexer->Source;
    return lexer;
}

void DeleteLexer(
    THIS PLEXER l
)
{ free(l); }

static TOKEN ReadTokenOnce(THIS PLEXER l);

TOKEN ReadTokenDirect(
    THIS PLEXER l
)
{
    for (;;) {
        l->CurrentToken = ReadTokenOnce(l);
        if (l->CurrentToken.Kind == TK_UNKNOWN) {
            LogErrorAtRange(
                &l->CurrentToken.Location,
                "stray '%c' in program",
                l->CurrentToken.Value.OffendingChar
            );
        }
        else if (l->CurrentToken.Kind != TK_COMMENT) {

            return l->CurrentToken;
        }
    }
}

void FreeToken(
    THIS PTOKEN t
)
{
    if (t->Kind == TK_IDENTIFIER)
        free(t->Value.IdentifierName);
    else if (t->Kind == TK_STR_CONSTANT)
        free(t->Value.StringValue);
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
        BOOL isOnlyWhitespace = TRUE;
        int lengthWithoutNewLine = firstCharLength;

        while (l->Cursor[lengthWithoutNewLine] != '\n') {
            if (!IsWhitespace(l->Cursor[lengthWithoutNewLine])) {
                isOnlyWhitespace = FALSE;
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
        l->CurrentFlags |= TOKENFLAG_BOL;
    }
    else if (trailingWhitespaceLength < 0) {
        l->CurrentLocation.Column += charLength;

        if (!IsWhitespace(charValue))
            l->CurrentFlags &= ~TOKENFLAG_BOL;
    }
    else {
        if (trailingWhitespaceLength > 0) {
            LogWarningAt(
                &l->CurrentLocation,
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
    IN   PTOKEN        t,
    OUT  PSOURCE_RANGE range
)
{
    int line = t->Location.Base.Line;
    int column = t->Location.Base.Column;
    const char *base = &l->Source->Lines[line][column];

    range->Base = t->Location.Base;
    range->Length = (int) (l->Cursor - base);
}

/* Precondition: GetChar(l) is [_A-Za-z] */
static void ReadIdentifier(
    THIS PLEXER l,
    OUT  PTOKEN t
)
{
    PSOURCE_FILE source = t->Location.Base.Source;

    int line   = t->Location.Base.Line,
        column = t->Location.Base.Column,
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
             if (o("if"))       t->Kind = TK_PP_if;
        else if (o("ifdef"))    t->Kind = TK_PP_ifdef;
        else if (o("ifndef"))   t->Kind = TK_PP_ifndef;
        else if (o("elif"))     t->Kind = TK_PP_elif;
        else if (o("endif"))    t->Kind = TK_PP_endif;
        else if (o("include"))  t->Kind = TK_PP_include;
        else if (o("define"))   t->Kind = TK_PP_define;
        else if (o("undef"))    t->Kind = TK_PP_undef;
        else if (o("line"))     t->Kind = TK_PP_line;
        else if (o("error"))    t->Kind = TK_PP_error;
        else if (o("warning"))  t->Kind = TK_PP_warning;
    }
    else if (o("const"))    t->Kind = TK_KW_const;
    else if (o("extern"))   t->Kind = TK_KW_extern;
    else if (o("static"))   t->Kind = TK_KW_static;
    else if (o("auto"))     t->Kind = TK_KW_auto;
    else if (o("volatile")) t->Kind = TK_KW_volatile;
    else if (o("unsigned")) t->Kind = TK_KW_unsigned;
    else if (o("signed"))   t->Kind = TK_KW_signed;
    else if (o("void"))     t->Kind = TK_KW_void;
    else if (o("char"))     t->Kind = TK_KW_char;
    else if (o("short"))    t->Kind = TK_KW_short;
    else if (o("int"))      t->Kind = TK_KW_int;
    else if (o("long"))     t->Kind = TK_KW_long;
    else if (o("float"))    t->Kind = TK_KW_float;
    else if (o("double"))   t->Kind = TK_KW_double;
    else if (o("enum"))     t->Kind = TK_KW_enum;
    else if (o("struct"))   t->Kind = TK_KW_struct;
    else if (o("union"))    t->Kind = TK_KW_union;
    else if (o("typedef"))  t->Kind = TK_KW_typedef;
    else if (o("sizeof"))   t->Kind = TK_KW_sizeof;
    else if (o("register")) t->Kind = TK_KW_register;
    else if (o("goto"))     t->Kind = TK_KW_goto;
    else if (o("if"))       t->Kind = TK_KW_if;
    else if (o("else"))     t->Kind = TK_KW_else;
    else if (o("switch"))   t->Kind = TK_KW_switch;
    else if (o("case"))     t->Kind = TK_KW_case;
    else if (o("default"))  t->Kind = TK_KW_default;
    else if (o("do"))       t->Kind = TK_KW_do;
    else if (o("while"))    t->Kind = TK_KW_while;
    else if (o("for"))      t->Kind = TK_KW_for;
    else if (o("break"))    t->Kind = TK_KW_break;
    else if (o("continue")) t->Kind = TK_KW_continue;
    else if (o("return"))   t->Kind = TK_KW_return;
    else {
        t->Kind = TK_IDENTIFIER;
        t->Value.IdentifierName = calloc(length + 1, sizeof(char));
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

    *suffix = calloc(*length + 1, sizeof(char));
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
    THIS PLEXER l,
    IN   PTOKEN t
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

        LogErrorAtRange(
            &range, "invalid suffix \"%s\" on integer constant", suffix
        );
    }

    free(suffix);
}

static void ReadFractionalLiteral(
    THIS PLEXER l,
    IN   PTOKEN t
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
        t->Kind = TK_FLOAT_CONSTANT;
        t->Value.FloatValue = t->Value.IntValue + floatFrac;
    }
    else {
        t->Kind = TK_DOUBLE_CONSTANT;
        t->Value.DoubleValue = t->Value.IntValue + doubleFrac;
    }

    if (suffixLength > 1) {
        SOURCE_RANGE range;
        GetTokenRange(l, t, &range);

        LogErrorAtRange(
            &range,
            "invalid suffix \"%s\" on floating constant",
            suffix
        );
    }

    free(suffix);
}

static void ReadHexLiteral(
    THIS PLEXER l,
    OUT  PTOKEN t
)
{
    t->Kind = TK_INT_CONSTANT;
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
    THIS PLEXER l,
    OUT  PTOKEN t
)
{
    t->Kind = TK_INT_CONSTANT;
    t->Value.IntValue = 0;
    for (; IsDecimal(GetChar(l)); IncrementCursor(l)) {
        if (IsOctal(GetChar(l))) {
            t->Value.IntValue = (t->Value.IntValue * 8) - (GetChar(l) - '0');
        }
        else {
            SOURCE_RANGE range;
            GetTokenRange(l, t, &range);

            LogErrorAtRange(
                &range,
                "invalid digit \"%c\" in octal constant",
                GetChar(l)
            );
        }
    }
    SkipIntSuffixes(l, t);
}

static void ReadDecimalLiteral(
    THIS PLEXER l,
    OUT  PTOKEN t
)
{
    t->Kind = TK_INT_CONSTANT;
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
    THIS PLEXER l,
    OUT  PTOKEN t
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

        errorRange.Base = l->CurrentLocation;
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
                LogErrorAtRange(
                    &errorRange,
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
    THIS PLEXER l,
    OUT  PTOKEN t
)
{
    SOURCE_RANGE range;

    IncrementCursor(l);
    t->Kind = TK_INT_CONSTANT;

    if (GetChar(l) == '\n') {
        GetTokenRange(l, t, &range);
        LogErrorAtRange(&range, "missing terminating ' character");
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
                    LogErrorAtRange(&range, "missing terminating ' character");
                    return;
                }

                IncrementCursor(l);
            }

            IncrementCursor(l);

            GetTokenRange(l, t, &range);
            LogWarningAtRange(
                &range,
                "character constant too long for its type"
            );
        }
    }
}

/* Precondition: GetChar(l) == '"' */
static void ReadStringLiteral(
    THIS PLEXER l,
    OUT  PTOKEN t
)
{
    int length = 0;
    int capacity = 12;

    t->Kind = TK_STR_CONSTANT;
    IncrementCursor(l);

    t->Value.StringValue = calloc(capacity + 1, sizeof(char));
    t->Value.StringValue[0] = 0;

    while (GetChar(l) != '"') {
        if (GetChar(l) == '\n') {
            SOURCE_RANGE range;
            GetTokenRange(l, t, &range);

            LogErrorAtRange(&range, "missing terminating \" character");

            if (t->Value.StringValue != NULL)
                free(t->Value.StringValue);

            t->Value.StringValue = NULL;
            return;
        }

        if (length >= capacity) {
            capacity *= 2;
            t->Value.StringValue = realloc(t->Value.StringValue, capacity + 1);
        }

        t->Value.StringValue[length] = ReadCharEscapeSequence(l);
        ++length;
    }

    IncrementCursor(l);
    t->Value.StringValue[length] = 0;
}

static TOKEN ReadTokenOnce(THIS PLEXER l)
{
    TOKEN result = { 0 };

    while (IsWhitespace(GetChar(l)))
        IncrementCursor(l);

    if (l->CurrentFlags & TOKENFLAG_BOL)
        l->CurrentMode = LM_DEFAULT;

    result.Flags = l->CurrentFlags;
    result.Location.Base = l->CurrentLocation;

    if ((result.Flags & TOKENFLAG_BOL) && GetChar(l) == '#') {
        IncrementCursor(l);
        result.Kind = TK_PP_HASH;
    }
    else {
        switch (GetChar(l)) {
        case 0: result.Kind = TK_EOF; break;
        case '(': IncrementCursor(l); result.Kind = TK_LPAREN;    break;
        case ')': IncrementCursor(l); result.Kind = TK_RPAREN;    break;
        case '[': IncrementCursor(l); result.Kind = TK_LBRACKET;  break;
        case ']': IncrementCursor(l); result.Kind = TK_RBRACKET;  break;
        case '{': IncrementCursor(l); result.Kind = TK_LBRACE;    break;
        case '}': IncrementCursor(l); result.Kind = TK_RBRACE;    break;
        case ';': IncrementCursor(l); result.Kind = TK_SEMICOLON; break;
        case ',': IncrementCursor(l); result.Kind = TK_COMMA;     break;
        case '~': IncrementCursor(l); result.Kind = TK_TILDE;     break;
        case '?': IncrementCursor(l); result.Kind = TK_QUESTION;  break;
        case ':': IncrementCursor(l); result.Kind = TK_COLON;     break;

        case '.':
            IncrementCursor(l);
            if (IsDecimal(GetChar(l))) {
                result.Value.IntValue = 0;
                ReadFractionalLiteral(l, &result);
            }
            else {
                result.Kind = TK_DOT;
            }
            break;

        case '+':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Kind = TK_PLUS_EQUALS;
            }
            else if (GetChar(l) == '+') {
                IncrementCursor(l);
                result.Kind = TK_PLUS_PLUS;
            }
            else {
                result.Kind = TK_PLUS;
            }
            break;

        case '-':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Kind = TK_MINUS_EQUALS;
            }
            else if (GetChar(l) == '-') {
                IncrementCursor(l);
                result.Kind = TK_MINUS_MINUS;
            }
            else if (GetChar(l) == '>') {
                IncrementCursor(l);
                result.Kind = TK_MINUS_GT;
            }
            else {
                result.Kind = TK_MINUS;
            }
            break;

        case '*':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Kind = TK_ASTERISK_EQUALS;
            }
            else {
                result.Kind = TK_ASTERISK;
            }
            break;

        case '/':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Kind = TK_SLASH_EQUALS;
            }
            else if (GetChar(l) == '*') {
                IncrementCursor(l);
                result.Kind = TK_COMMENT;

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
                            LogErrorAtRange(&range, "unterminated comment");
                            break;
                        }
                    }
                    else if (GetChar(l) == 0) {
                        GetTokenRange(l, &result, &range);
                        LogErrorAtRange(&range, "unterminated comment");
                        break;
                    }
                    else {
                        IncrementCursor(l);
                    }
                }
            }
            else {
                result.Kind = TK_SLASH;
            }
            break;

        case '%':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Kind = TK_PERCENT_EQUALS;
            }
            else {
                result.Kind = TK_PERCENT;
            }
            break;

        case '<':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Kind = TK_LT_EQUALS;
            }
            else if (GetChar(l) == '<') {
                IncrementCursor(l);
                if (GetChar(l) == '=') {
                    IncrementCursor(l);
                    result.Kind = TK_LT_LT_EQUALS;
                }
                else {
                    result.Kind = TK_LT_LT;
                }
            }
            else {
                result.Kind = TK_LT;
            }
            break;

        case '>':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Kind = TK_GT_EQUALS;
            }
            else if (GetChar(l) == '>') {
                IncrementCursor(l);
                if (GetChar(l) == '=') {
                    IncrementCursor(l);
                    result.Kind = TK_GT_GT_EQUALS;
                }
                else {
                    result.Kind = TK_GT_GT;
                }
            }
            else {
                result.Kind = TK_GT;
            }
            break;

        case '=':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Kind = TK_EQUALS_EQUALS;
            }
            else {
                result.Kind = TK_EQUALS;
            }
            break;

        case '!':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Kind = TK_EXCLAMATION_EQUALS;
            }
            else {
                result.Kind = TK_EXCLAMATION;
            }
            break;

        case '&':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Kind = TK_AMPERSAND_EQUALS;
            }
            else if (GetChar(l) == '&') {
                IncrementCursor(l);
                result.Kind = TK_AMPERSAND_AMPERSAND;
            }
            else {
                result.Kind = TK_AMPERSAND;
            }
            break;

        case '^':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Kind = TK_CARET_EQUALS;
            }
            else {
                result.Kind = TK_CARET;
            } 
            break;

        case '|':
            IncrementCursor(l);
            if (GetChar(l) == '=') {
                IncrementCursor(l);
                result.Kind = TK_PIPE_EQUALS;
            }
            else if (GetChar(l) == '|') {
                IncrementCursor(l);
                result.Kind = TK_PIPE_PIPE;
            }
            else {
                result.Kind = TK_PIPE;
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
            result.Kind = TK_UNKNOWN;
            result.Value.OffendingChar = GetChar(l);
            IncrementCursor(l);
            break;
        }
    }

    if (result.Kind == TK_COMMENT) {
        int line = result.Location.Base.Line;
        int column = result.Location.Base.Column;
        const char *base = &l->Source->Lines[line][column];
        result.Location.Length = (int) (l->Cursor - base);
    }
    else {
        int end = l->CurrentLocation.Column;
        int start = result.Location.Base.Column;
        result.Location.Length = end - start;
    }

    if (l->CurrentMode == LM_DEFAULT && result.Kind == TK_PP_HASH) {
        l->CurrentMode |= LM_PP_DIRECTIVE;
        l->CurrentMode |= LM_PP_DIRECTIVE_KW;
    }
    else if (l->CurrentMode & LM_PP_DIRECTIVE_KW) {
        l->CurrentMode &= ~LM_PP_DIRECTIVE_KW;

        if (result.Kind == TK_PP_include)
            l->CurrentMode |= LM_PP_ANGLED_STRING_CONSTANT;
    }
    else if (l->CurrentMode & LM_PP_ANGLED_STRING_CONSTANT) {
        l->CurrentMode &= ~LM_PP_ANGLED_STRING_CONSTANT;
    }

    return result;
}
