#define _CRT_SECURE_NO_WARNINGS
#include "source.hh"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SourceFile::SourceFile(const std::string& fileName) :
    FileName{ fileName },
    Data{ nullptr },
    Lines{ nullptr },
    IsOpen{ false }
{
    FILE* file{ fopen(fileName.c_str(), "rb") };
    if (!file) {
        return;
    }

    fseek(file, 0, SEEK_END);

    int length{ static_cast<int>(ftell(file)) };
    fseek(file, 0, SEEK_SET);
    Data = new char[length + 3]{ };
    fread(Data, length, 1, file);
    Data[length] = '\n';
    Data[length + 1] = '\n';
    Data[length + 2] = 0;
    fclose(file);

    int lineCount{ 1 };
    for (int i{ 0 }; i < length; ++i) {
        if (Data[i] == '\n')
            ++lineCount;
    }

    Lines = new char*[lineCount]{ };
    lineCount = 0;
    Lines[0] = Data;
    for (int i{ 0 }; i < length; ++i) {
        if (Data[i] == '\n')
            Lines[++lineCount] = &Data[i + 1];
    }

    for (int i{ 0 }; Data[i] != 0; ++i) {
        Contents.push_back(Data[i]);
    }
    Contents.push_back(0);
    IsOpen = true;
}

SourceFile::~SourceFile() {
    delete[] Lines;
    delete[] Data;
}
