#ifndef PTC_CL_TEXT_DECODER_H
#define PTC_CL_TEXT_DECODER_H
#include <stdint.h>

typedef uint32_t   UTF32CHAR;
typedef UTF32CHAR *UTF32STRING;

typedef struct TEXT_DECODER TEXT_DECODER, *PTEXT_DECODER;

PTEXT_DECODER CreateTextDecoder(const char *text);
void          DeleteTextDecoder(PTEXT_DECODER obj);
void          GetDecodedString(PTEXT_DECODER obj, UTF32STRING *outString, int *outLength);

#endif
