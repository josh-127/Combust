#include "source.hh"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SourceFile::SourceFile(IN const char* fileName) :
    FileName{ nullptr },
    Contents{ nullptr },
    Lines{ nullptr },
    IsOpen{ false }
{
    FILE* file{ fopen(fileName, "rb") };
    if (!file) {
        return;
    }

    fseek(file, 0, SEEK_END);

    int length{ static_cast<int>(ftell(file)) };
    fseek(file, 0, SEEK_SET);
    Contents = new char[length + 3]{ };
    fread(Contents, length, 1, file);
    Contents[length] = '\n';
    Contents[length + 1] = '\n';
    Contents[length + 2] = 0;
    fclose(file);

    int lineCount{ 1 };
    for (int i{ 0 }; i < length; ++i) {
        if (Contents[i] == '\n')
            ++lineCount;
    }

    Lines = new char*[lineCount]{ };
    lineCount = 0;
    Lines[0] = Contents;
    for (int i{ 0 }; i < length; ++i) {
        if (Contents[i] == '\n')
            Lines[++lineCount] = &Contents[i + 1];
    }

    FileName = new char[strlen(fileName) + 1]{ };
    strcpy(FileName, fileName);

    IsOpen = true;
}

SourceFile::~SourceFile() {
    delete[] Lines;
    delete[] Contents;
    delete[] FileName;
}
