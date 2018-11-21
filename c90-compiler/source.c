#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int OpenSourceFile(const char *fileName, PSOURCE_FILE sourceFile) {
    FILE        *file;
    int          length;
    int          lineCount;
    int          i;

    file = fopen(fileName, "rb");
    if (!file) {
        return -1;
    }

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);
    sourceFile->Contents = calloc(length + 2, sizeof(char));
    fread(sourceFile->Contents, length, 1, file);
    sourceFile->Contents[length] = '\n';
    sourceFile->Contents[length + 1] = 0;
    fclose(file);

    lineCount = 1;
    for (i = 0; i < length; ++i) {
        if (sourceFile->Contents[i] == '\n')
            ++lineCount;
    }

    sourceFile->Lines = calloc(lineCount, sizeof(char *));
    lineCount = 0;
    sourceFile->Lines[0] = sourceFile->Contents;
    for (i = 0; i < length; ++i) {
        if (sourceFile->Contents[i] == '\n')
            sourceFile->Lines[++lineCount] = &sourceFile->Contents[i + 1];
    }

    sourceFile->FileName = calloc(strlen(fileName) + 1, sizeof(char));
    strcpy(sourceFile->FileName, fileName);

    return 0;
}

void CloseSourceFile(PSOURCE_FILE obj) {
    free(obj->Lines);
    free(obj->Contents);
    free(obj->FileName);
}
