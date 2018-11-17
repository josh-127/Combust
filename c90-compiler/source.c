#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SOURCE_FILE OpenSourceFile(const char *file_name) {
    FILE *file;
    SOURCE_FILE result;
    unsigned length;
    unsigned lineCount;
    unsigned i;

    file = fopen(file_name, "rb");
    if (!file) {
        /* TODO: fatal error */
    }

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);
    result.Contents = malloc(length + 2);
    fread(result.Contents, length, 1, file);
    result.Contents[length] = '\n';
    result.Contents[length + 1] = 0;
    fclose(file);

    lineCount = 1;
    for (i = 0; i < length; ++i) {
        if (result.Contents[i] == '\n')
            ++lineCount;
    }

    result.Lines = malloc(lineCount * sizeof(char*));
    lineCount = 0;
    result.Lines[0] = result.Contents;
    for (i = 0; i < length; ++i) {
        if (result.Contents[i] == '\n')
            result.Lines[++lineCount] = &result.Contents[i + 1];
    }

    result.FileName = malloc(strlen(file_name) + 1);
    strcpy(result.FileName, file_name);
    return result;
}

void CloseSourceFile(PSOURCE_FILE sf) {
    free(sf->Lines);
    free(sf->Contents);
    free(sf->FileName);
}
