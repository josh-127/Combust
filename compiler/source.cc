#include "source.hh"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int OpenSourceFile(
    IN  const char   *fileName,
    OUT PSOURCE_FILE  sourceFile
)
{
    FILE* file{ fopen(fileName, "rb") };
    if (!file) {
        return -1;
    }

    fseek(file, 0, SEEK_END);

    int length{ static_cast<int>(ftell(file)) };
    fseek(file, 0, SEEK_SET);
    sourceFile->Contents = new char[length + 3]{ };
    fread(sourceFile->Contents, length, 1, file);
    sourceFile->Contents[length] = '\n';
    sourceFile->Contents[length + 1] = '\n';
    sourceFile->Contents[length + 2] = 0;
    fclose(file);

    int lineCount{ 1 };
    for (int i{ 0 }; i < length; ++i) {
        if (sourceFile->Contents[i] == '\n')
            ++lineCount;
    }

    sourceFile->Lines = new char*[lineCount]{ };
    lineCount = 0;
    sourceFile->Lines[0] = sourceFile->Contents;
    for (int i{ 0 }; i < length; ++i) {
        if (sourceFile->Contents[i] == '\n')
            sourceFile->Lines[++lineCount] = &sourceFile->Contents[i + 1];
    }

    sourceFile->FileName = new char[strlen(fileName) + 1]{ };
    strcpy(sourceFile->FileName, fileName);

    return 0;
}

void CloseSourceFile(
    THIS PSOURCE_FILE obj
)
{
    delete[] obj->Lines;
    delete[] obj->Contents;
    delete[] obj->FileName;
}
