#include "source.hh"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int OpenSourceFile(
    IN  const char   *fileName,
    OUT PSOURCE_FILE  sourceFile
)
{
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
    sourceFile->Contents = static_cast<char*>(calloc(length + 3, sizeof(char)));
    fread(sourceFile->Contents, length, 1, file);
    sourceFile->Contents[length] = '\n';
    sourceFile->Contents[length + 1] = '\n';
    sourceFile->Contents[length + 2] = 0;
    fclose(file);

    lineCount = 1;
    for (i = 0; i < length; ++i) {
        if (sourceFile->Contents[i] == '\n')
            ++lineCount;
    }

    sourceFile->Lines = static_cast<char**>(calloc(lineCount, sizeof(char *)));
    lineCount = 0;
    sourceFile->Lines[0] = sourceFile->Contents;
    for (i = 0; i < length; ++i) {
        if (sourceFile->Contents[i] == '\n')
            sourceFile->Lines[++lineCount] = &sourceFile->Contents[i + 1];
    }

    sourceFile->FileName = static_cast<char*>(calloc(strlen(fileName) + 1, sizeof(char)));
    strcpy(sourceFile->FileName, fileName);

    return 0;
}

void CloseSourceFile(
    THIS PSOURCE_FILE obj
)
{
    free(obj->Lines);
    free(obj->Contents);
    free(obj->FileName);
}
