#include "text-decoder.h"
#include <stdlib.h>
#include <string.h>

struct TEXT_DECODER {
    UTF32STRING Result;
    int         ResultLength;
};

static int CountChars(const char *text);
static void DecodeChars(const char *source, UTF32STRING dest);

PTEXT_DECODER CreateTextDecoder(const char *text) {
    PTEXT_DECODER obj = malloc(sizeof(TEXT_DECODER));
    obj->ResultLength = CountChars(text);

    obj->Result = malloc((obj->ResultLength + 1) * sizeof(UTF32CHAR));
    DecodeChars(text, obj->Result);

    return obj;
}

void DeleteTextDecoder(PTEXT_DECODER obj) {
    free(obj->Result);
    free(obj);
}

void GetDecodedString(PTEXT_DECODER obj, UTF32STRING *outString, int *outLength) {
    if (outString != NULL)
        *outString = obj->Result;

    if (outLength != NULL)
        *outLength = obj->ResultLength;
}

static int CountChars(const char *text) {
    const char *cursor = text;
    int count = 0;

    while (*cursor != 0) {
        if (cursor[0] == '?' && cursor[1] == '?' &&
            (
                cursor[2] == '=' ||
                cursor[2] == '(' ||
                cursor[2] == '/' ||
                cursor[2] == ')' ||
                cursor[2] == '\'' ||
                cursor[2] == '<' ||
                cursor[2] == '!' ||
                cursor[2] == '>' ||
                cursor[2] == '-'
            )
        )
        {
            cursor += 3;
        }
        else {
            ++cursor;
        }

        ++count;
    }

    return count;
}

static void DecodeChars(const char *source, UTF32STRING dest) {
    const char *sourceCursor = source;
    UTF32CHAR  *destCursor = dest;

    while (*sourceCursor != 0) {
        if (sourceCursor[0] == '?' && sourceCursor[1] == '?') {
            switch (sourceCursor[2]) {
                case '=':  *destCursor = '#';  sourceCursor += 3; ++destCursor; continue;
                case '(':  *destCursor = '[';  sourceCursor += 3; ++destCursor; continue;
                case '/':  *destCursor = '\\'; sourceCursor += 3; ++destCursor; continue;
                case ')':  *destCursor = ']';  sourceCursor += 3; ++destCursor; continue;
                case '\'': *destCursor = '^';  sourceCursor += 3; ++destCursor; continue;
                case '<':  *destCursor = '{';  sourceCursor += 3; ++destCursor; continue;
                case '!':  *destCursor = '|';  sourceCursor += 3; ++destCursor; continue;
                case '>':  *destCursor = '}';  sourceCursor += 3; ++destCursor; continue;
                case '-':  *destCursor = '~';  sourceCursor += 3; ++destCursor; continue;
            }
        }

        *destCursor = *sourceCursor;
        ++sourceCursor;
        ++destCursor;
    }

    *destCursor = 0;
}
