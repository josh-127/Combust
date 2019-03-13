#ifndef COMBUST_SYNTAX_HH
#define COMBUST_SYNTAX_HH
#include "common.hh"
#include "source.hh"
#include <stdint.h>
#include <string>
#include <type_traits>

constexpr int ST_BEGINNING_OF_LINE{ 1 };


class SyntaxNode;
class SyntaxToken;
#define Tk(className) class className
#include "syntax-kinds.def"
#undef Tk
class StrayToken;
class CommentToken;
class IdentifierToken;
class NumericLiteralToken;
class CharLiteralToken;
class StringConstantToken;
class AngledStringConstantToken;


class SyntaxNodeVisitor : public Object {
public:
#define Tk(className) virtual Rc<Object> Visit(className& obj) = 0
#include "syntax-kinds.def"
#undef Tk
    virtual Rc<Object> Visit(StrayToken& obj) = 0;
    virtual Rc<Object> Visit(CommentToken& obj) = 0;
    virtual Rc<Object> Visit(IdentifierToken& obj) = 0;
    virtual Rc<Object> Visit(NumericLiteralToken& obj) = 0;
    virtual Rc<Object> Visit(CharLiteralToken& obj) = 0;
    virtual Rc<Object> Visit(StringConstantToken& obj) = 0;
    virtual Rc<Object> Visit(AngledStringConstantToken& obj) = 0;
};


class SyntaxNode : public Object {
public:
    const SourceRange& GetLexemeRange() const { return lexemeRange; }
    void SetLexemeRange(const SourceRange& to) { lexemeRange = to; }

    virtual Rc<Object> Accept(SyntaxNodeVisitor& visitor) = 0;
protected:
    explicit SyntaxNode() {}
    virtual ~SyntaxNode() {}
private:
    SourceRange lexemeRange{ };
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
        Rc<Object> Accept(SyntaxNodeVisitor& visitor) override; \
    }
#include "syntax-kinds.def"
#undef Tk

class StrayToken : public SyntaxToken {
public:
    explicit StrayToken() {}
    virtual ~StrayToken() {}
    char GetOffendingChar() const { return offendingChar; }
    void SetOffendingChar(const char to) { offendingChar = to; }
    Rc<Object> Accept(SyntaxNodeVisitor& visitor) override { return visitor.Visit(*this); }
private:
    char offendingChar{ 0 };
};

class CommentToken : public SyntaxToken {
public:
    explicit CommentToken() {}
    virtual ~CommentToken() {}
    Rc<Object> Accept(SyntaxNodeVisitor& visitor) override { return visitor.Visit(*this); }
};

class IdentifierToken : public SyntaxToken {
public:
    explicit IdentifierToken() {}
    virtual ~IdentifierToken() {}
    const std::string& GetName() const { return name; }
    void SetName(const std::string& to) { name = to; }
    Rc<Object> Accept(SyntaxNodeVisitor& visitor) override { return visitor.Visit(*this); }
private:
    std::string name{ };
};

class NumericLiteralToken : public SyntaxToken {
public:
    explicit NumericLiteralToken() {}
    virtual ~NumericLiteralToken() {}
    const std::string& GetWholeValue() const { return wholeValue; }
    void SetWholeValue(const std::string& to) { wholeValue = to; }
    const std::string& GetFractionalValue() const { return fractionalValue; }
    void SetFractionalValue(const std::string& to) { fractionalValue = to; }
    const std::string& GetDotSymbol() const { return dotSymbol; }
    void SetDotSymbol(const std::string& to) { dotSymbol = to; }
    const std::string& GetPrefix() const { return prefix; }
    void SetPrefix(const std::string& to) { prefix = to; }
    const std::string& GetSuffix() const { return suffix; }
    void SetSuffix(const std::string& to) { suffix = to; }
    Rc<Object> Accept(SyntaxNodeVisitor& visitor) override { return visitor.Visit(*this); }
private:
    std::string wholeValue{ };
    std::string fractionalValue{ };
    std::string dotSymbol{ };
    std::string prefix{ };
    std::string suffix{ };
};

class CharLiteralToken : public SyntaxToken {
public:
    explicit CharLiteralToken() {}
    virtual ~CharLiteralToken() {}
    const std::string& GetContents() const { return contents; }
    void SetContents(const std::string& to) { contents = to; }
    char GetOpeningQuote() const { return openingQuote; }
    void SetOpeningQuote(const char to) { openingQuote = to; }
    char GetClosingQuote() const { return closingQuote; }
    void SetClosingQuote(const char to) { closingQuote = to; }
    Rc<Object> Accept(SyntaxNodeVisitor& visitor) override { return visitor.Visit(*this); }
private:
    std::string contents{ };
    char openingQuote{ 0 };
    char closingQuote{ 0 };
};

class StringConstantToken : public SyntaxToken {
public:
    explicit StringConstantToken() {}
    virtual ~StringConstantToken() {}
    const std::string& GetValue() const { return value; }
    void SetValue(const std::string& to) { value = to; }
    Rc<Object> Accept(SyntaxNodeVisitor& visitor) override { return visitor.Visit(*this); }
private:
    std::string value{ };
};

class AngledStringConstantToken : public SyntaxToken {
public:
    explicit AngledStringConstantToken() {}
    virtual ~AngledStringConstantToken() {}
    const std::string& GetValue() const { return value; }
    void SetValue(const std::string& to) { value = to; }
    Rc<Object> Accept(SyntaxNodeVisitor& visitor) override { return visitor.Visit(*this); }
private:
    std::string value{ };
};


template<typename T>
class IsTokenVisitor : public SyntaxNodeVisitor {
public:
    explicit IsTokenVisitor() {}
    virtual ~IsTokenVisitor() {}
    bool GetResult() const { return result; }

#define O(className)                                     \
    Rc<Object> Visit(className& obj) override {          \
        (void) obj;                                      \
        if constexpr (std::is_same<T, className>::value) \
            result = true;                               \
        return Rc<Object>{ };                            \
    }

#define Tk(className) O(className)
#include "syntax-kinds.def"
#undef Tk

    O(StrayToken)
    O(CommentToken)
    O(IdentifierToken)
    O(NumericLiteralToken)
    O(CharLiteralToken)
    O(StringConstantToken)
    O(AngledStringConstantToken)
#undef O

private:
    bool result{ false };
};

template<typename T>
[[nodiscard]] inline bool IsToken(Rc<SyntaxToken> token) {
    IsTokenVisitor<T> visitorFunction{ };
    token->Accept(visitorFunction);
    return visitorFunction.GetResult();
}

#endif
