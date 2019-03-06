#define _CRT_SECURE_NO_WARNINGS
#include "source.hh"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SourceFile::SourceFile(const std::string& fileName) :
    FileName{ fileName },
    IsOpen{ false }
{
    FILE* file{ fopen(fileName.c_str(), "rb") };
    if (!file) {
        return;
    }

    fseek(file, 0, SEEK_END);

    int length{ static_cast<int>(ftell(file)) };

    fseek(file, 0, SEEK_SET);

    char* data{ new char[length + 1]{ } };
    fread(data, length, 1, file);

    fclose(file);

    for (size_t i{ 0 }; data[i] != 0; ++i) {
        Contents.push_back(data[i]);
    }
    Contents.push_back('\n');
    Contents.push_back('\n');
    Contents.push_back(0);

    for (size_t i{ 0 }; i < Contents.size(); ++i) {
        char c{ Contents[i] };
        if (c == '\n') {
            Lines.push_back(i);
        }
    }

    IsOpen = true;

    delete[] data;
}

SourceFile::~SourceFile() {}
