#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SOURCE_FILE OpenSourceFile(const char *file_name)
{
    FILE *file;
    SOURCE_FILE result;
    unsigned length;
    unsigned line_count;
    unsigned i;

    file = fopen(file_name, "rb");
    if (!file) {
        /* TODO: fatal error */
    }

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);
    result.Contents = malloc(length + 1);
    fread(result.Contents, length, 1, file);
    result.Contents[length] = 0;
    fclose(file);

    line_count = 1;
    for (i = 0; i < length; ++i) {
        if (result.Contents[i] == '\n')
            ++line_count;
    }

    result.Lines = malloc(line_count * sizeof(char*));
    line_count = 0;
    result.Lines[0] = result.Contents;
    for (i = 0; i < length; ++i) {
        if (result.Contents[i] == '\n')
            result.Lines[++line_count] = &result.Contents[i + 1];
    }

    result.FileName = malloc(strlen(file_name) + 1);
    strcpy(result.FileName, file_name);
    return result;
}

void CloseSourceFile(SOURCE_FILE *sf)
{
    free(sf->Lines);
    free(sf->Contents);
    free(sf->FileName);
}
