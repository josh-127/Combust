#include "lexer.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { false, true = !false } bool;

struct LEXER {
    PSOURCE_FILE Source;
    char        *Cursor;
    LEXER_MODE   CurrentModes;
    int          CurrentFlags;
    SOURCE_LOC   CurrentLocation;
    TOKEN        CurrentToken;
};

PLEXER CreateLexer(PSOURCE_FILE input)
{
    PLEXER lexer = malloc(sizeof(LEXER));
    lexer->Source                 = input;
    lexer->Cursor                 = lexer->Source->Contents;
    lexer->CurrentModes           = LM_DEFAULT;
    lexer->CurrentFlags           = TOKENFLAG_BOL;
    lexer->CurrentLocation.Line   = 0;
    lexer->CurrentLocation.Column = 0;
    lexer->CurrentLocation.Source = lexer->Source;
    return lexer;
}

void DeleteLexer(PLEXER l)
{ free(l); }

void EnableLexerMode(PLEXER l, LEXER_MODE modes)
{ l->CurrentModes |= modes; }

void DisableLexerMode(PLEXER l, LEXER_MODE modes)
{ l->CurrentModes &= ~modes; }

static TOKEN ScanToken(PLEXER l);

TOKEN ReadTokenDirect(PLEXER l)
{
    for (;;) {
        l->CurrentToken = ScanToken(l);
        if (l->CurrentToken.Kind == TK_UNKNOWN) {
            LogErrorC(&l->CurrentToken.Location, "stray '%c' in program",
                      l->CurrentToken.Value.OffendingChar);
        }
        else if (l->CurrentToken.Kind != TK_COMMENT) {
            return l->CurrentToken;
        }
    }
}

void FreeToken(PTOKEN t)
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

static void IncrementCursor(PLEXER l)
{
    if (*l->Cursor == '\n') {
        ++l->CurrentLocation.Line;
        l->CurrentLocation.Column = -1;
    }

    ++l->Cursor;
    ++l->CurrentLocation.Column;
}

static void IncrementCursorBy(PLEXER l, unsigned amount)
{
    while (amount--)
        IncrementCursor(l);
}

/* Precondition: *l->Cursor is [_A-Za-z] */
static void ScanIdentifier(PLEXER l, PTOKEN t)
{
    unsigned length = 0;

    while ((l->Cursor[length] == '_') ||
           (l->Cursor[length] == '$') ||
           (l->Cursor[length] >= 'A' && l->Cursor[length] <= 'Z') ||
           (l->Cursor[length] >= 'a' && l->Cursor[length] <= 'z') ||
           (l->Cursor[length] >= '0' && l->Cursor[length] <= '9'))
    {
        ++length;
    }

#define o(kw) (length == ((sizeof(kw) - 1) / sizeof(char)) && !strncmp(l->Cursor, kw, length))
    if (l->CurrentModes & LM_PP_DIRECTIVE_KW) {
             if (o("if"))       t->Kind = TK_PP_IF;
        else if (o("ifdef"))    t->Kind = TK_PP_IFDEF;
        else if (o("ifndef"))   t->Kind = TK_PP_IFNDEF;
        else if (o("elif"))     t->Kind = TK_PP_ELIF;
        else if (o("endif"))    t->Kind = TK_PP_ENDIF;
        else if (o("include"))  t->Kind = TK_PP_INCLUDE;
        else if (o("define"))   t->Kind = TK_PP_DEFINE;
        else if (o("undef"))    t->Kind = TK_PP_UNDEF;
        else if (o("line"))     t->Kind = TK_PP_LINE;
        else if (o("error"))    t->Kind = TK_PP_ERROR;
        else if (o("warning"))  t->Kind = TK_PP_WARNING;
    }
    else if (o("const"))    t->Kind = TK_KW_CONST;
    else if (o("extern"))   t->Kind = TK_KW_EXTERN;
    else if (o("static"))   t->Kind = TK_KW_STATIC;
    else if (o("auto"))     t->Kind = TK_KW_AUTO;
    else if (o("volatile")) t->Kind = TK_KW_VOLATILE;
    else if (o("unsigned")) t->Kind = TK_KW_UNSIGNED;
    else if (o("signed"))   t->Kind = TK_KW_SIGNED;
    else if (o("void"))     t->Kind = TK_KW_VOID;
    else if (o("char"))     t->Kind = TK_KW_CHAR;
    else if (o("short"))    t->Kind = TK_KW_SHORT;
    else if (o("int"))      t->Kind = TK_KW_INT;
    else if (o("long"))     t->Kind = TK_KW_LONG;
    else if (o("float"))    t->Kind = TK_KW_FLOAT;
    else if (o("double"))   t->Kind = TK_KW_DOUBLE;
    else if (o("enum"))     t->Kind = TK_KW_ENUM;
    else if (o("struct"))   t->Kind = TK_KW_STRUCT;
    else if (o("union"))    t->Kind = TK_KW_UNION;
    else if (o("typedef"))  t->Kind = TK_KW_TYPEDEF;
    else if (o("sizeof"))   t->Kind = TK_KW_SIZEOF;
    else if (o("register")) t->Kind = TK_KW_REGISTER;
    else if (o("goto"))     t->Kind = TK_KW_GOTO;
    else if (o("if"))       t->Kind = TK_KW_IF;
    else if (o("else"))     t->Kind = TK_KW_ELSE;
    else if (o("switch"))   t->Kind = TK_KW_SWITCH;
    else if (o("case"))     t->Kind = TK_KW_CASE;
    else if (o("default"))  t->Kind = TK_KW_DEFAULT;
    else if (o("do"))       t->Kind = TK_KW_DO;
    else if (o("while"))    t->Kind = TK_KW_WHILE;
    else if (o("for"))      t->Kind = TK_KW_FOR;
    else if (o("break"))    t->Kind = TK_KW_BREAK;
    else if (o("continue")) t->Kind = TK_KW_CONTINUE;
    else if (o("return"))   t->Kind = TK_KW_RETURN;
    else {
        t->Kind = TK_IDENTIFIER;
        t->Value.IdentifierName = malloc(length + 1);
        strncpy(t->Value.IdentifierName, l->Cursor, length);
        t->Value.IdentifierName[length] = 0;
    }
#undef o

    IncrementCursorBy(l, length);
}

static void ScanSuffix(PLEXER l, char **suffix, unsigned *length)
{
    unsigned i;

    *length = 0;
    while (IsLetter(l->Cursor[*length]))
        ++*length;

    *suffix = malloc(*length + 1);
    for (i = 0; i < *length; ++i)
        (*suffix)[i] = l->Cursor[i];
    (*suffix)[*length] = 0;
    IncrementCursorBy(l, *length);
}

static int SkipUnsignedSuffix(char **cursor)
{
    if (**cursor == 'U' || **cursor == 'u') {
        ++*cursor;
        return 1;
    }
    return 0;
}

static int SkipLongSuffix(char **cursor)
{
    if (**cursor == 'L' || **cursor == 'l') {
        ++*cursor;
        if (**cursor == 'L' || **cursor == 'l')
            ++*cursor;
        return 1;
    }
    return 0;
}

static void SkipIntSuffixes(PLEXER l, PTOKEN t)
{
    unsigned suffixLength = 0;
    char *suffix;
    char *suffixCursor;

    ScanSuffix(l, &suffix, &suffixLength);
    suffixCursor = suffix;

    if (SkipUnsignedSuffix(&suffixCursor))
        SkipLongSuffix(&suffixCursor);
    else if (SkipLongSuffix(&suffixCursor))
        SkipUnsignedSuffix(&suffixCursor);

    if (IsLetter(*suffixCursor)) {
        LogErrorC(&t->Location, "invalid suffix \"%s\" on integer constant", suffix);
    }

    free(suffix);
}

static void ScanFractionalLiteral(PLEXER l, PTOKEN t)
{
    float floatFrac = 0.0F;
    float floatExp = 0.1F;
    float doubleFrac = 0.0;
    float doubleExp = 0.1;
    char *suffix;
    unsigned suffixLength;

    for (; IsDecimal(*l->Cursor); IncrementCursor(l)) {
        floatFrac += floatExp * (*l->Cursor - '0');
        doubleFrac += doubleExp * (*l->Cursor - '0');
        floatExp *= 0.1F;
        doubleExp *= 0.1;
    }

    ScanSuffix(l, &suffix, &suffixLength);

    if (*suffix == 'f' || *suffix == 'F') {
        t->Kind = TK_FLOAT_CONSTANT;
        t->Value.FloatValue = t->Value.IntValue + floatFrac;
    }
    else {
        t->Kind = TK_DOUBLE_CONSTANT;
        t->Value.DoubleValue = t->Value.IntValue + doubleFrac;
    }

    if (suffixLength > 1) {
        LogErrorC(&t->Location, "invalid suffix \"%s\" on floating constant", suffix);
    }

    free(suffix);
}

static void ScanHexLiteral(PLEXER l, PTOKEN t)
{
    t->Kind = TK_INT_CONSTANT;
    t->Value.IntValue = 0;
    while (IsHex(*l->Cursor)) {
        t->Value.IntValue *= 16;
        if (*l->Cursor <= '9')
            t->Value.IntValue += *l->Cursor - '0';
        else if (*l->Cursor <= 'F')
            t->Value.IntValue += *l->Cursor - 'A' + 10;
        else
            t->Value.IntValue += *l->Cursor - 'a' + 10;

        IncrementCursor(l);
    }

    SkipIntSuffixes(l, t);
}

static void ScanOctalLiteral(PLEXER l, PTOKEN t)
{
    t->Kind = TK_INT_CONSTANT;
    t->Value.IntValue = 0;
    for (; IsDecimal(*l->Cursor); IncrementCursor(l)) {
        if (IsOctal(*l->Cursor)) {
            t->Value.IntValue = (t->Value.IntValue * 8) - (*l->Cursor - '0');
        }
        else {
            LogErrorC(&t->Location, "invalid digit \"%c\" in octal constant", *l->Cursor);
        }
    }
    SkipIntSuffixes(l, t);
}

static void ScanDecimalLiteral(PLEXER l, PTOKEN t)
{
    t->Kind = TK_INT_CONSTANT;
    t->Value.IntValue = 0;
    for (; IsDecimal(*l->Cursor); IncrementCursor(l))
        t->Value.IntValue = (t->Value.IntValue * 10) + (*l->Cursor - '0');

    if (*l->Cursor == '.') {
        IncrementCursor(l);
        ScanFractionalLiteral(l, t);
    }
    else {
        SkipIntSuffixes(l, t);
    }
}

static void ScanNumericalLiteral(PLEXER l, PTOKEN t)
{
    if (*l->Cursor == '0') {
        unsigned wholeLength = 0;

        do {
            IncrementCursor(l);
        }
        while (*l->Cursor == '0');

        while (IsDecimal(l->Cursor[wholeLength]))
            ++wholeLength;

        if (l->Cursor[wholeLength] == '.') {
            ScanDecimalLiteral(l, t);
        }
        else if (*l->Cursor == 'X' || *l->Cursor == 'x') {
            IncrementCursor(l);
            ScanHexLiteral(l, t);
        }
        else {
            ScanOctalLiteral(l, t);
        }
    }
    else {
        ScanDecimalLiteral(l, t);
    }
}

static char ScanChar(PLEXER l)
{
    char result;

    if (*l->Cursor == '\\') {
        unsigned digitCount;

        IncrementCursor(l);

        switch (*l->Cursor) {
        case '\'':  IncrementCursor(l); result = '\''; break;
        case '"':   IncrementCursor(l); result = '\"'; break;
        case '?':   IncrementCursor(l); result = '\?'; break;
        case '\\':  IncrementCursor(l); result = '\\'; break;
        case 'a':   IncrementCursor(l); result = '\a'; break;
        case 'b':   IncrementCursor(l); result = '\b'; break;
        case 'f':   IncrementCursor(l); result = '\f'; break;
        case 'n':   IncrementCursor(l); result = '\n'; break;
        case 'r':   IncrementCursor(l); result = '\r'; break;
        case 't':   IncrementCursor(l); result = '\t'; break;
        case 'v':   IncrementCursor(l); result = '\v'; break;

        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
            result = 0;
            digitCount = 0;
            while (digitCount < 3 && IsOctal(*l->Cursor)) {
                result = (result * 8) + (*l->Cursor - '0');
                ++digitCount;
                IncrementCursor(l);
            }
            break;

        case 'x':
            IncrementCursor(l);
            result = 0;
            for (; IsHex(*l->Cursor); IncrementCursor(l)) {
                result *= 16;
                if (*l->Cursor <= '9')      result += *l->Cursor - '0';
                else if (*l->Cursor <= 'F') result += *l->Cursor - 'A' + 10;
                else if (*l->Cursor <= 'f') result += *l->Cursor - 'a' + 10;
            }
            break;
        }
    }
    else {
        result = *l->Cursor;
        IncrementCursor(l);
    }

    return result;
}

/* Precondition: *l->Cursor == '\'' */
static void ScanCharLiteral(PLEXER l, PTOKEN t)
{
    IncrementCursor(l);
    t->Kind = TK_INT_CONSTANT;

    if (*l->Cursor == '\n') {
        LogErrorC(&t->Location, "missing terminating ' character");
    }
    else {
        t->Value.IntValue = ScanChar(l);

        if (*l->Cursor == '\'') {
            IncrementCursor(l);
        }
        else {
            while (*l->Cursor != '\'') {
                if (*l->Cursor == '\n') {
                    LogErrorC(&t->Location, "missing terminating ' character");
                    return;
                }

                IncrementCursor(l);
            }

            IncrementCursor(l);
            LogWarningC(&t->Location, "character constant too long for its type");
        }
    }
}

static void CountUnescapedChar(PLEXER l, unsigned *length)
{
    if (l->Cursor[*length] == '\\') {
        unsigned j;
        ++*length;

        switch (l->Cursor[*length]) {
        case '\'':  case '"':   case '?':
        case '\\':  case 'a':   case 'b':
        case 'f':   case 'n':   case 'r':
        case 't':   case 'v':
            ++*length;
            break;

        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
            for (j = 0; j < 3 && IsOctal(l->Cursor[*length]); ++j)
                ++*length;
            break;

        case 'x':
            ++*length;
            while (IsHex(l->Cursor[*length]))
                ++*length;
            break;
        }
    }
    else {
        ++*length;
    }
}

/* Precondition: *l->Cursor == '"' */
static void ScanStringLiteral(PLEXER l, PTOKEN t)
{
    unsigned length = 0;
    unsigned unescapedLength = 1;
    unsigned i;

    t->Kind = TK_STR_CONSTANT;

    while (l->Cursor[unescapedLength] != '"') {
        if (l->Cursor[unescapedLength] == '\n') {
            LogErrorC(&t->Location, "missing terminating \" character");
            IncrementCursorBy(l, unescapedLength);
            t->Value.StringValue = 0;
            return;
        }

        CountUnescapedChar(l, &unescapedLength);
        ++length;
    }

    t->Value.StringValue = malloc(length + 1);

    IncrementCursor(l);
    for (i = 0; *l->Cursor != '"'; ++i)
        t->Value.StringValue[i] = ScanChar(l);

    t->Value.StringValue[length] = 0;
    IncrementCursor(l);
}

static TOKEN ScanToken(PLEXER l)
{
    TOKEN result;

    while (IsWhitespace(*l->Cursor)) {
        if (*l->Cursor == '\n')
            l->CurrentFlags |= TOKENFLAG_BOL;
        IncrementCursor(l);
    }

    if (l->CurrentModes & LM_PP_DIRECTIVE && *l->Cursor == '\\') {
        bool is_only_whitespace = true;
        unsigned chars_before_newline = 0;

        while (l->Cursor[chars_before_newline + 1] != '\n') {
            if (!IsWhitespace(l->Cursor[chars_before_newline + 1])) {
                is_only_whitespace = false;
                break;
            }
            ++chars_before_newline;
        }

        if (is_only_whitespace) {
            if (chars_before_newline) {
                LogWarningC(&l->CurrentLocation, "backslash and newline separated by space");
            }
            IncrementCursorBy(l, chars_before_newline + 1);
            while (IsWhitespace(*l->Cursor))
                IncrementCursor(l);
        }
    }

    result.Flags = l->CurrentFlags;
    result.Location = l->CurrentLocation;

    if (result.Flags & TOKENFLAG_BOL && *l->Cursor == '#') {
        IncrementCursor(l);
        result.Kind = TK_PP_HASH;
    }
    else {
        switch (*l->Cursor) {
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
            if (IsDecimal(*l->Cursor)) {
                result.Value.IntValue = 0;
                ScanFractionalLiteral(l, &result);
            }
            else {
                result.Kind = TK_DOT;
            }
            break;

        case '+':
            IncrementCursor(l);
            if (*l->Cursor == '=') {
                IncrementCursor(l);
                result.Kind = TK_PLUS_EQUALS;
            }
            else if (*l->Cursor == '+') {
                IncrementCursor(l);
                result.Kind = TK_PLUS_PLUS;
            }
            else {
                result.Kind = TK_PLUS;
            }
            break;

        case '-':
            IncrementCursor(l);
            if (*l->Cursor == '=') {
                IncrementCursor(l);
                result.Kind = TK_MINUS_EQUALS;
            }
            else if (*l->Cursor == '-') {
                IncrementCursor(l);
                result.Kind = TK_MINUS_MINUS;
            }
            else if (*l->Cursor == '>') {
                IncrementCursor(l);
                result.Kind = TK_MINUS_GT;
            }
            else {
                result.Kind = TK_MINUS;
            }
            break;

        case '*':
            IncrementCursor(l);
            if (*l->Cursor == '=') {
                IncrementCursor(l);
                result.Kind = TK_ASTERISK_EQUALS;
            }
            else {
                result.Kind = TK_ASTERISK;
            }
            break;

        case '/':
            IncrementCursor(l);
            if (*l->Cursor == '=') {
                IncrementCursor(l);
                result.Kind = TK_SLASH_EQUALS;
            }
            else if (*l->Cursor == '*') {
                IncrementCursor(l);
                result.Kind = TK_COMMENT;

                for (;;) {
                    if (*l->Cursor == '*') {
                        IncrementCursor(l);

                        if (*l->Cursor == '/') {
                            IncrementCursor(l);
                            break;
                        }
                        else if (*l->Cursor == 0) {
                            LogErrorC(&result.Location, "unterminated comment");
                            break;
                        }
                    }
                    else if (*l->Cursor == 0) {
                        LogErrorC(&result.Location, "unterminated comment");
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
            if (*l->Cursor == '=') {
                IncrementCursor(l);
                result.Kind = TK_PERCENT_EQUALS;
            }
            else {
                result.Kind = TK_PERCENT;
            }
            break;

        case '<':
            IncrementCursor(l);
            if (*l->Cursor == '=') {
                IncrementCursor(l);
                result.Kind = TK_LT_EQUALS;
            }
            else if (*l->Cursor == '<') {
                IncrementCursor(l);
                if (*l->Cursor == '=') {
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
            if (*l->Cursor == '=') {
                IncrementCursor(l);
                result.Kind = TK_GT_EQUALS;
            }
            else if (*l->Cursor == '>') {
                IncrementCursor(l);
                if (*l->Cursor == '=') {
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
            if (*l->Cursor == '=') {
                IncrementCursor(l);
                result.Kind = TK_EQUALS_EQUALS;
            }
            else {
                result.Kind = TK_EQUALS;
            }
            break;

        case '!':
            IncrementCursor(l);
            if (*l->Cursor == '=') {
                IncrementCursor(l);
                result.Kind = TK_EXCLAMATION_EQUALS;
            }
            else {
                result.Kind = TK_EXCLAMATION;
            }
            break;

        case '&':
            IncrementCursor(l);
            if (*l->Cursor == '=') {
                IncrementCursor(l);
                result.Kind = TK_AMPERSAND_EQUALS;
            }
            else if (*l->Cursor == '&') {
                IncrementCursor(l);
                result.Kind = TK_AMPERSAND_AMPERSAND;
            }
            else {
                result.Kind = TK_AMPERSAND;
            }
            break;

        case '^':
            IncrementCursor(l);
            if (*l->Cursor == '=') {
                IncrementCursor(l);
                result.Kind = TK_CARET_EQUALS;
            }
            else {
                result.Kind = TK_CARET;
            } 
            break;

        case '|':
            IncrementCursor(l);
            if (*l->Cursor == '=') {
                IncrementCursor(l);
                result.Kind = TK_PIPE_EQUALS;
            }
            else if (*l->Cursor == '|') {
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
            ScanIdentifier(l, &result);
            break;

        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        case '8': case '9':
            ScanNumericalLiteral(l, &result);
            break;

        case '\'':
            ScanCharLiteral(l, &result);
            break;

        case '"':
            ScanStringLiteral(l, &result);
            break;

        default:
            result.Kind = TK_UNKNOWN;
            result.Value.OffendingChar = *l->Cursor;
            IncrementCursor(l);
            break;
        }
    }

    if (result.Kind == TK_COMMENT) {
        result.Length = (unsigned) (l->Cursor - &l->Source->Lines[result.Location.Line][result.Location.Column]);
    }
    else {
        result.Length = l->CurrentLocation.Column - result.Location.Column;
    }

    l->CurrentFlags &= ~TOKENFLAG_BOL;
    return result;
}
