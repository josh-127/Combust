#include "common.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { false, true = !false } bool;

struct Lexer {
    struct SourceFile* Source;
    char*              Cursor;
    enum LexerModes    CurrentModes;
    int                CurrentFlags;
    struct SourceLoc   CurrentLocation;
    struct Token       CurrentToken;
};

Lexer* create_lexer(struct SourceFile* input)
{
    Lexer* lexer = malloc(sizeof(Lexer));
    lexer->Source                 = input;
    lexer->Cursor                 = lexer->Source->Contents;
    lexer->CurrentModes           = LM_DEFAULT;
    lexer->CurrentFlags           = TOKENFLAG_BOL;
    lexer->CurrentLocation.Line   = 0;
    lexer->CurrentLocation.Column = 0;
    lexer->CurrentLocation.Source = lexer->Source;
    return lexer;
}

void delete_lexer(Lexer* l)
{ free(l); }

void enable_lexer_modes(Lexer* l, enum LexerModes modes)
{ l->CurrentModes |= modes; }

void disable_lexer_modes(Lexer* l, enum LexerModes modes)
{ l->CurrentModes &= ~modes; }

static struct Token scan_token(Lexer* l);

struct Token peek_token_direct(Lexer* l)
{ return l->CurrentToken; }

struct Token read_token_direct(Lexer* l)
{
    for (;;) {
        l->CurrentToken = scan_token(l);
        if (l->CurrentToken.Kind == TK_UNKNOWN) {
            log_error_c(&l->CurrentToken.Location, "stray '%c' in program",
                        l->CurrentToken.OffendingChar);
        }
        else if (l->CurrentToken.Kind != TK_COMMENT) {
            return l->CurrentToken;
        }
    }
}

void free_token(struct Token* t)
{
    if (t->Kind == TK_IDENTIFIER)
        free(t->IdentifierName);
    else if (t->Kind == TK_STR_CONSTANT)
        free(t->StringValue);
}

#define is_whitespace(c) ((c) == ' ' || \
                          (c) == '\n' || \
                          (c) == '\r' || \
                          (c) == '\t')

#define is_letter(c) ((c) >= 'A' && (c) <= 'Z' || \
                      (c) >= 'a' && (c) <= 'z')

#define is_decimal(c) ((c) >= '0' && (c) <= '9')

#define is_octal(c) ((c) >= '0' && (c) <= '7')

#define is_hex(c) ((c) >= '0' && (c) <= '9' || \
                   (c) >= 'A' && (c) <= 'F' || \
                   (c) >= 'a' && (c) <= 'f')

static void increment_cursor(Lexer* l)
{
    ++l->Cursor;
    ++l->CurrentLocation.Column;
}

static void increment_cursor_by(Lexer* l, unsigned amount)
{
    while (amount--)
        increment_cursor(l);
}

static void increment_cursor_multiline(Lexer* l)
{
    if (*l->Cursor == '\n') {
        ++l->CurrentLocation.Line;
        l->CurrentLocation.Column = -1;
        increment_cursor(l);
    }
    else {
        increment_cursor(l);
    }
}

/* Precondition: *l->Cursor is [_A-Za-z] */
static void scan_identifier(Lexer* l, struct Token* t)
{
    unsigned length = 0;

    while (l->Cursor[length] == '_' || l->Cursor[length] == '$' ||
           l->Cursor[length] >= 'A' && l->Cursor[length] <= 'Z' ||
           l->Cursor[length] >= 'a' && l->Cursor[length] <= 'z' ||
           l->Cursor[length] >= '0' && l->Cursor[length] <= '9')
    {
        ++length;
    }

#define o(kw) !strncmp(l->Cursor, kw, length)
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
        t->IdentifierName = malloc(length + 1);
        strncpy(t->IdentifierName, l->Cursor, length);
        t->IdentifierName[length] = 0;
    }
#undef o

    increment_cursor_by(l, length);
}

static void scan_suffix(Lexer* l, char** suffix, unsigned* length)
{
    unsigned i;

    *length = 0;
    while (is_letter(l->Cursor[*length]))
        ++*length;

    *suffix = malloc(*length + 1);
    for (i = 0; i < *length; ++i)
        (*suffix)[i] = l->Cursor[i];
    (*suffix)[*length] = 0;
    increment_cursor_by(l, *length);
}

static int skip_unsigned_suffix(char** cursor)
{
    if (**cursor == 'U' || **cursor == 'u') {
        ++*cursor;
        return 1;
    }
    return 0;
}

static int skip_long_suffix(char** cursor)
{
    if (**cursor == 'L' || **cursor == 'l') {
        ++*cursor;
        if (**cursor == 'L' || **cursor == 'l')
            ++*cursor;
        return 1;
    }
    return 0;
}

static void skip_int_suffixes(Lexer* l, struct Token* t)
{
    unsigned suffix_length = 0;
    char* suffix;
    char* suffix_cursor;
    unsigned i;

    scan_suffix(l, &suffix, &suffix_length);
    suffix_cursor = suffix;

    if (skip_unsigned_suffix(&suffix_cursor))
        skip_long_suffix(&suffix_cursor);
    else if (skip_long_suffix(&suffix_cursor))
        skip_unsigned_suffix(&suffix_cursor);

    if (is_letter(*suffix_cursor)) {
        log_error_c(&t->Location, "invalid suffix \"%s\" on integer constant", suffix);
    }

    free(suffix);
}

static void scan_fractional_literal(Lexer* l, struct Token* t)
{
    float float_frac = 0.0F;
    float float_exp = 0.1F;
    float double_frac = 0.0;
    float double_exp = 0.1;
    char* suffix;
    unsigned suffix_length;

    for (; is_decimal(*l->Cursor); increment_cursor(l)) {
        float_frac += float_exp * (*l->Cursor - '0');
        double_frac += double_exp * (*l->Cursor - '0');
        float_exp *= 0.1F;
        double_exp *= 0.1;
    }

    scan_suffix(l, &suffix, &suffix_length);

    if (*suffix == 'f' || *suffix == 'F') {
        t->Kind = TK_FLOAT_CONSTANT;
        t->FloatValue = t->IntValue + float_frac;
    }
    else {
        t->Kind = TK_DOUBLE_CONSTANT;
        t->DoubleValue = t->IntValue + double_frac;
    }

    if (suffix[1] != 0) {
        log_error_c(&t->Location, "invalid suffix \"%s\" on floating constant", suffix);
    }

    free(suffix);
}

static void scan_hex_literal(Lexer* l, struct Token* t)
{
    t->Kind = TK_INT_CONSTANT;
    t->IntValue = 0;
    while (is_hex(*l->Cursor)) {
        t->IntValue *= 16;
        if (*l->Cursor <= '9')
            t->IntValue += *l->Cursor - '0';
        else if (*l->Cursor <= 'F')
            t->IntValue += *l->Cursor - 'A' + 10;
        else
            t->IntValue += *l->Cursor - 'a' + 10;

        increment_cursor(l);
    }

    skip_int_suffixes(l, t);
}

static void scan_octal_literal(Lexer* l, struct Token* t)
{
    t->Kind = TK_INT_CONSTANT;
    t->IntValue = 0;
    for (; is_decimal(*l->Cursor); increment_cursor(l)) {
        if (is_octal(*l->Cursor)) {
            t->IntValue = (t->IntValue * 8) - (*l->Cursor - '0');
        }
        else {
            log_error_c(&t->Location, "invalid digit \"%c\" in octal constant", *l->Cursor);
        }
    }
    skip_int_suffixes(l, t);
}

static void scan_decimal_literal(Lexer* l, struct Token* t)
{
    t->Kind = TK_INT_CONSTANT;
    t->IntValue = 0;
    for (; is_decimal(*l->Cursor); increment_cursor(l))
        t->IntValue = (t->IntValue * 10) + (*l->Cursor - '0');

    if (*l->Cursor == '.') {
        increment_cursor(l);
        scan_fractional_literal(l, t);
    }
    else {
        skip_int_suffixes(l, t);
    }
}

static void scan_numerical_literal(Lexer* l, struct Token* t)
{
    if (*l->Cursor == '0') {
        unsigned whole_length = 0;

        do {
            increment_cursor(l);
        }
        while (*l->Cursor == '0');

        while (is_decimal(l->Cursor[whole_length]))
            ++whole_length;

        if (l->Cursor[whole_length] == '.') {
            scan_decimal_literal(l, t);
        }
        else if (*l->Cursor == 'X' || *l->Cursor == 'x') {
            increment_cursor(l);
            scan_hex_literal(l, t);
        }
        else {
            scan_octal_literal(l, t);
        }
    }
    else {
        scan_decimal_literal(l, t);
    }
}

static char scan_char(Lexer* l)
{
    char result;

    if (*l->Cursor == '\\') {
        unsigned digit_count;

        increment_cursor(l);

        switch (*l->Cursor) {
        case '\'':  increment_cursor(l); result = '\''; break;
        case '"':   increment_cursor(l); result = '\"'; break;
        case '?':   increment_cursor(l); result = '\?'; break;
        case '\\':  increment_cursor(l); result = '\\'; break;
        case 'a':   increment_cursor(l); result = '\a'; break;
        case 'b':   increment_cursor(l); result = '\b'; break;
        case 'f':   increment_cursor(l); result = '\f'; break;
        case 'n':   increment_cursor(l); result = '\n'; break;
        case 'r':   increment_cursor(l); result = '\r'; break;
        case 't':   increment_cursor(l); result = '\t'; break;
        case 'v':   increment_cursor(l); result = '\v'; break;

        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
            result = 0;
            digit_count = 0;
            while (digit_count < 3 && is_octal(*l->Cursor)) {
                result = (result * 8) + (*l->Cursor - '0');
                ++digit_count;
                increment_cursor(l);
            }
            break;

        case 'x':
            increment_cursor(l);
            result = 0;
            for (; is_hex(*l->Cursor); increment_cursor(l)) {
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
        increment_cursor(l);
    }

    return result;
}

/* Precondition: *l->Cursor == '\'' */
static void scan_char_literal(Lexer* l, struct Token* t)
{
    increment_cursor(l);
    t->Kind = TK_INT_CONSTANT;

    if (*l->Cursor == '\n') {
        log_error_c(&t->Location, "missing terminating ' character");
    }
    else {
        t->IntValue = scan_char(l);

        if (*l->Cursor == '\'') {
            increment_cursor(l);
        }
        else {
            /* TODO: Can hit null terminator without newline at
                     end of file */
            while (*l->Cursor != '\'') {
                if (*l->Cursor == '\n') {
                    log_error_c(&t->Location, "missing terminating ' character");
                    return;
                }

                increment_cursor(l);
            }

            increment_cursor(l);
            log_warning_c(&t->Location, "character constant too long for its type");
        }
    }
}

static void count_unescaped_char(Lexer* l, unsigned* length)
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
            for (j = 0; j < 3, is_octal(l->Cursor[*length]); ++j)
                ++*length;
            break;

        case 'x':
            ++*length;
            while (is_hex(l->Cursor[*length]))
                ++*length;
            break;
        }
    }
    else {
        ++*length;
    }
}

/* Precondition: *l->Cursor == '"' */
static void scan_string_literal(Lexer* l, struct Token* t)
{
    unsigned length = 0;
    unsigned unescaped_length = 1;
    unsigned i;

    t->Kind = TK_STR_CONSTANT;

    /* TODO: Can hit a null terminator and go out of bounds */
    while (l->Cursor[unescaped_length] != '"') {
        if (l->Cursor[unescaped_length] == '\n') {
            log_error_c(&t->Location, "missing terminating \" character");
            increment_cursor_by(l, unescaped_length);
            t->StringValue = 0;
            return;
        }

        count_unescaped_char(l, &unescaped_length);
        ++length;
    }

    t->StringValue = malloc(length + 1);

    increment_cursor(l);
    for (i = 0; *l->Cursor != '"'; ++i)
        t->StringValue[i] = scan_char(l);

    t->StringValue[length] = 0;
    increment_cursor(l);
}

static struct Token scan_token(Lexer* l)
{
    struct Token result;

    while (is_whitespace(*l->Cursor)) {
        if (*l->Cursor == '\n')
            l->CurrentFlags |= TOKENFLAG_BOL;
        increment_cursor_multiline(l);
    }

    if (l->CurrentModes & LM_PP_DIRECTIVE && *l->Cursor == '\\') {
        bool is_only_whitespace = true;
        unsigned chars_before_newline = 0;

        while (l->Cursor[chars_before_newline + 1] != '\n') {
            if (!is_whitespace(l->Cursor[chars_before_newline + 1])) {
                is_only_whitespace = false;
                break;
            }
            ++chars_before_newline;
        }

        if (is_only_whitespace) {
            if (chars_before_newline) {
                log_warning_c(&l->CurrentLocation, "backslash and newline separated by space");
            }
            increment_cursor_by(l, chars_before_newline + 1);
            while (is_whitespace(*l->Cursor))
                increment_cursor_multiline(l);
        }
    }

    result.Flags = l->CurrentFlags;
    result.Location = l->CurrentLocation;

    if (result.Flags & TOKENFLAG_BOL && *l->Cursor == '#') {
        increment_cursor(l);
        result.Kind = TK_PP_HASH;
    }
    else {
        switch (*l->Cursor) {
        case 0: result.Kind = TK_EOF; break;
        case '(': increment_cursor(l); result.Kind = TK_LPAREN;    break;
        case ')': increment_cursor(l); result.Kind = TK_RPAREN;    break;
        case '[': increment_cursor(l); result.Kind = TK_LBRACKET;  break;
        case ']': increment_cursor(l); result.Kind = TK_RBRACKET;  break;
        case '{': increment_cursor(l); result.Kind = TK_LBRACE;    break;
        case '}': increment_cursor(l); result.Kind = TK_RBRACE;    break;
        case ';': increment_cursor(l); result.Kind = TK_SEMICOLON; break;
        case ',': increment_cursor(l); result.Kind = TK_COMMA;     break;
        case '~': increment_cursor(l); result.Kind = TK_TILDE;     break;
        case '?': increment_cursor(l); result.Kind = TK_QUESTION;  break;
        case ':': increment_cursor(l); result.Kind = TK_COLON;     break;

        case '.':
            increment_cursor(l);
            if (is_decimal(*l->Cursor)) {
                result.IntValue = 0;
                scan_fractional_literal(l, &result);
            }
            else {
                result.Kind = TK_DOT;
            }
            break;

        case '+':
            increment_cursor(l);
            if (*l->Cursor == '=') {
                increment_cursor(l);
                result.Kind = TK_PLUS_EQUALS;
            }
            else if (*l->Cursor == '+') {
                increment_cursor(l);
                result.Kind = TK_PLUS_PLUS;
            }
            else {
                result.Kind = TK_PLUS;
            }
            break;

        case '-':
            increment_cursor(l);
            if (*l->Cursor == '=') {
                increment_cursor(l);
                result.Kind = TK_MINUS_EQUALS;
            }
            else if (*l->Cursor == '-') {
                increment_cursor(l);
                result.Kind = TK_MINUS_MINUS;
            }
            else if (*l->Cursor == '>') {
                increment_cursor(l);
                result.Kind = TK_MINUS_GT;
            }
            else {
                result.Kind = TK_MINUS;
            }
            break;

        case '*':
            increment_cursor(l);
            if (*l->Cursor == '=') {
                increment_cursor(l);
                result.Kind = TK_ASTERISK_EQUALS;
            }
            else {
                result.Kind = TK_ASTERISK;
            }
            break;

        case '/':
            increment_cursor(l);
            if (*l->Cursor == '=') {
                increment_cursor(l);
                result.Kind = TK_SLASH_EQUALS;
            }
            else if (*l->Cursor == '*') {
                increment_cursor(l);
                result.Kind = TK_COMMENT;

                if (*l->Cursor == '/')
                    increment_cursor(l);

                while (*l->Cursor != '/') {
                    while (*l->Cursor != '*')
                        increment_cursor_multiline(l);
                    increment_cursor(l);
                }

                increment_cursor(l);
            }
            else {
                result.Kind = TK_SLASH;
            }
            break;

        case '%':
            increment_cursor(l);
            if (*l->Cursor == '=') {
                increment_cursor(l);
                result.Kind = TK_PERCENT_EQUALS;
            }
            else {
                result.Kind = TK_PERCENT;
            }
            break;

        case '<':
            increment_cursor(l);
            if (*l->Cursor == '=') {
                increment_cursor(l);
                result.Kind = TK_LT_EQUALS;
            }
            else if (*l->Cursor == '<') {
                increment_cursor(l);
                if (*l->Cursor == '=') {
                    increment_cursor(l);
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
            increment_cursor(l);
            if (*l->Cursor == '=') {
                increment_cursor(l);
                result.Kind = TK_GT_EQUALS;
            }
            else if (*l->Cursor == '>') {
                increment_cursor(l);
                if (*l->Cursor == '=') {
                    increment_cursor(l);
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
            increment_cursor(l);
            if (*l->Cursor == '=') {
                increment_cursor(l);
                result.Kind = TK_EQUALS_EQUALS;
            }
            else {
                result.Kind = TK_EQUALS;
            }
            break;

        case '!':
            increment_cursor(l);
            if (*l->Cursor == '=') {
                increment_cursor(l);
                result.Kind = TK_EXCLAMATION_EQUALS;
            }
            else {
                result.Kind = TK_EXCLAMATION;
            }
            break;

        case '&':
            increment_cursor(l);
            if (*l->Cursor == '=') {
                increment_cursor(l);
                result.Kind = TK_AMPERSAND_EQUALS;
            }
            else if (*l->Cursor == '&') {
                increment_cursor(l);
                result.Kind = TK_AMPERSAND_AMPERSAND;
            }
            else {
                result.Kind = TK_AMPERSAND;
            }
            break;

        case '^':
            increment_cursor(l);
            if (*l->Cursor == '=') {
                increment_cursor(l);
                result.Kind = TK_CARET_EQUALS;
            }
            else {
                result.Kind = TK_CARET;
            } 
            break;

        case '|':
            increment_cursor(l);
            if (*l->Cursor == '=') {
                increment_cursor(l);
                result.Kind = TK_PIPE_EQUALS;
            }
            else if (*l->Cursor == '|') {
                increment_cursor(l);
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
            scan_identifier(l, &result);
            break;

        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        case '8': case '9':
            scan_numerical_literal(l, &result);
            break;

        case '\'':
            scan_char_literal(l, &result);
            break;

        case '"':
            scan_string_literal(l, &result);
            break;

        default:
            result.Kind = TK_UNKNOWN;
            result.OffendingChar = *l->Cursor;
            increment_cursor(l);
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

