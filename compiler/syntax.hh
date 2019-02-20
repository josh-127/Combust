#ifndef PTC_CL_SYNTAX_H
#define PTC_CL_SYNTAX_H
#include "common.hh"
#include "source.hh"
#include <stdint.h>
#include <string>

constexpr int ST_BEGINNING_OF_LINE{ 1 };


class SyntaxNode;
class SyntaxToken;
#define Tk(className) class className
#include "syntax-kinds.def"
#undef Tk
class StrayToken;
class CommentToken;
class IdentifierToken;
class IntConstantToken;
class FloatConstantToken;
class DoubleConstantToken;
class StringConstantToken;
class AngledStringConstantToken;


class SyntaxTokenVisitor : public Object {
public:
#define Tk(className) virtual void Visit(className& obj) = 0
#include "syntax-kinds.def"
#undef Tk
    virtual void Visit(StrayToken& obj) = 0;
    virtual void Visit(CommentToken& obj) = 0;
    virtual void Visit(IdentifierToken& obj) = 0;
    virtual void Visit(IntConstantToken& obj) = 0;
    virtual void Visit(FloatConstantToken& obj) = 0;
    virtual void Visit(DoubleConstantToken& obj) = 0;
    virtual void Visit(StringConstantToken& obj) = 0;
    virtual void Visit(AngledStringConstantToken& obj) = 0;
};


class SyntaxNode : public Object {
public:
    SOURCE_RANGE GetLexemeRange() const { return lexemeRange; }
    void SetLexemeRange(const SOURCE_RANGE& to) { lexemeRange = to; }

    virtual void Accept(SyntaxTokenVisitor& visitor) = 0;
protected:
    explicit SyntaxNode() {}
    virtual ~SyntaxNode() {}
private:
    SOURCE_RANGE lexemeRange{ };
};

class SyntaxToken : public SyntaxNode {
public:
    uint32_t GetFlags() const { return flags; }
    void SetFlags(const uint32_t to) { flags = to; }
protected:
    explicit SyntaxToken() {}
    virtual ~SyntaxToken() {}
private:
    uint32_t    flags{ 0 };
};

#define Tk(className) \
    class className : public SyntaxToken {                 \
    public:                                                \
        explicit className();                              \
        virtual ~className();                              \
        void Accept(SyntaxTokenVisitor& visitor) override; \
    }
#include "syntax-kinds.def"
#undef Tk

class StrayToken : public SyntaxToken {
public:
    explicit StrayToken() {}
    virtual ~StrayToken() {}
    char GetOffendingChar() const { return offendingChar; }
    void SetOffendingChar(const char to) { offendingChar = to; }
    void Accept(SyntaxTokenVisitor& visitor) override { visitor.Visit(*this); }
private:
    char offendingChar{ 0 };
};

class CommentToken : public SyntaxToken {
public:
    explicit CommentToken() {}
    virtual ~CommentToken() {}
    virtual void Accept(SyntaxTokenVisitor& visitor) override { visitor.Visit(*this); }
};

class IdentifierToken : public SyntaxToken {
public:
    explicit IdentifierToken() {}
    virtual ~IdentifierToken() {}
    const std::string& GetName() const { return name; }
    void SetName(const std::string& to) { name = to; }
    void Accept(SyntaxTokenVisitor& visitor) override { visitor.Visit(*this); }
private:
    std::string name{ };
};

class IntConstantToken : public SyntaxToken {
public:
    explicit IntConstantToken() {}
    virtual ~IntConstantToken() {}
    long GetValue() const { return value; }
    void SetValue(const long to) { value = to; }
    void Accept(SyntaxTokenVisitor& visitor) override { visitor.Visit(*this); }
private:
    long value{ 0L };
};

class FloatConstantToken : public SyntaxToken {
public:
    explicit FloatConstantToken() {}
    virtual ~FloatConstantToken() {}
    float GetValue() const { return value; }
    void SetValue(const float to) { value = to; }
    void Accept(SyntaxTokenVisitor& visitor) override { visitor.Visit(*this); }
private:
    float value{ 0.0F };
};

class DoubleConstantToken : public SyntaxToken {
public:
    explicit DoubleConstantToken() {}
    virtual ~DoubleConstantToken() {}
    double GetValue() const { return value; }
    void SetValue(const double to) { value = to; }
    void Accept(SyntaxTokenVisitor& visitor) override { visitor.Visit(*this); }
private:
    double value{ 0.0 };
};

class StringConstantToken : public SyntaxToken {
public:
    explicit StringConstantToken() {}
    virtual ~StringConstantToken() {}
    const std::string& GetValue() const { return value; }
    void SetValue(const std::string& to) { value = to; }
    void Accept(SyntaxTokenVisitor& visitor) override { visitor.Visit(*this); }
private:
    std::string value{ };
};

class AngledStringConstantToken : public SyntaxToken {
public:
    explicit AngledStringConstantToken() {}
    virtual ~AngledStringConstantToken() {}
    const std::string& GetValue() const { return value; }
    void SetValue(const std::string& to) { value = to; }
    void Accept(SyntaxTokenVisitor& visitor) override { visitor.Visit(*this); }
private:
    std::string value{ };
};

#endif
