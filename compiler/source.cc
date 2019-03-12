#define _CRT_SECURE_NO_WARNINGS
#include "source.hh"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SourceFile::SourceFile(
    const std::string& name,
    const std::vector<char>& contents
) :
    Name{ name },
    Contents{ contents }
{
    for (size_t i{ 0 }; i < contents.size(); ++i) {
        if (contents[i] == '\n') {
            lines.push_back(i);
        }
    }
}

SourceFile::~SourceFile() { }

std::string SourceFile::GetLine(int line) const {
    int length{ 0 };

    for (int i{ lines[line] }; Contents[i] != '\n'; ++i)
        ++length;

    std::string lineContents{ };
    lineContents.reserve(length);

    for (int i{ lines[line] }; Contents[i] != '\n'; ++i)
        lineContents += Contents[i];

    return lineContents;
}

Rc<SourceFile> CreateSourceFile(
    const std::string& name,
    const std::string& contents
) {
    std::vector<char> contentsVec{ };
    contentsVec.reserve(contents.size());

    for (char c : contents)
        contentsVec.push_back(c);

    return NewObj<SourceFile>(name, contentsVec);
}

Rc<SourceFile> OpenSourceFile(const std::string& path) {
    FILE* file{ fopen(path.c_str(), "rb") };
    if (file == nullptr) {
        return Rc<SourceFile>{ };
    }

    fseek(file, 0, SEEK_END);

    int length{ static_cast<int>(ftell(file)) };

    fseek(file, 0, SEEK_SET);

    char* data{ new char[length + 1]{ } };
    fread(data, length, 1, file);

    fclose(file);

    std::vector<char> contents;
    contents.reserve(length);

    for (int i{ 0 }; data[i] != 0; ++i) {
        contents.push_back(data[i]);
    }
    contents.push_back('\n');
    contents.push_back('\n');
    contents.push_back(0);

    delete[] data;

    return NewObj<SourceFile>(path, contents);
}
