#include <gtest/gtest.h>
#include "../preprocessor-lexer.hh"
#include "../source.hh"
#include "../syntax.hh"

TEST(PreprocessorLexerTest, EmptyFile) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<EofToken>(token));
}

TEST(PreprocessorLexerTest, HStringLiteralToken) {
    std::string path{ "FooBar" };
    Rc<SourceFile> sourceFile{
        CreateSourceFile("", "#include <" + path + ">")
    };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> firstToken{ preprocessor->ReadToken() };
    EXPECT_TRUE(firstToken->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<IncludeDirective>(firstToken));

    Rc<SyntaxToken> secondToken{ preprocessor->ReadToken() };
    EXPECT_TRUE(!(secondToken->GetFlags() & SyntaxToken::BEGINNING_OF_LINE));
    ASSERT_TRUE(IsSyntaxNode<StringLiteralToken>(secondToken));

    Rc<StringLiteralToken> literalToken{ std::static_pointer_cast<StringLiteralToken>(secondToken) };
    EXPECT_EQ(literalToken->GetValue(), path);
    EXPECT_EQ(literalToken->GetOpeningQuote(), '<');
    EXPECT_EQ(literalToken->GetClosingQuote(), '>');
}

TEST(PreprocessorLexerTest, HStringLiteralToken_WithoutWhitespaceInBetween) {
    std::string path{ "FooBar" };
    Rc<SourceFile> sourceFile{
        CreateSourceFile("", "#include<" + path + ">")
    };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> firstToken{ preprocessor->ReadToken() };
    EXPECT_TRUE(firstToken->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<IncludeDirective>(firstToken));

    Rc<SyntaxToken> secondToken{ preprocessor->ReadToken() };
    EXPECT_TRUE(!(secondToken->GetFlags() & SyntaxToken::BEGINNING_OF_LINE));
    ASSERT_TRUE(IsSyntaxNode<StringLiteralToken>(secondToken));

    Rc<StringLiteralToken> literalToken{ std::static_pointer_cast<StringLiteralToken>(secondToken) };
    EXPECT_EQ(literalToken->GetValue(), path);
    EXPECT_EQ(literalToken->GetOpeningQuote(), '<');
    EXPECT_EQ(literalToken->GetClosingQuote(), '>');
}

TEST(PreprocessorLexerTest, IdentifierToken) {
    Rc<SourceFile> sourceFile{
        CreateSourceFile(
            "",
            "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
        )
    };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(token));
}

TEST(PreprocessorLexerTest, InvalidDirective) {
    std::string name{ "FooBar" };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", '#' + name) };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<InvalidDirective>(token));

    Rc<InvalidDirective> directive{ std::static_pointer_cast<InvalidDirective>(token) };
    EXPECT_EQ(directive->GetName(), name);
    EXPECT_TRUE(directive->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
}

TEST(PreprocessorLexerTest, InvalidDirective_WithWhitespaceInBetween) {
    std::string name{ "FooBar" };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   " + name) };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<InvalidDirective>(token));

    Rc<InvalidDirective> directive{ std::static_pointer_cast<InvalidDirective>(token) };
    EXPECT_EQ(directive->GetName(), name);
}

TEST(PreprocessorLexerTest, IfDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#if") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<IfDirective>(token));
}

TEST(PreprocessorLexerTest, IfDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   if") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<IfDirective>(token));
}

TEST(PreprocessorLexerTest, IfDefDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#ifdef") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<IfDefDirective>(token));
}

TEST(PreprocessorLexerTest, IfDefDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   ifdef") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<IfDefDirective>(token));
}

TEST(PreprocessorLexerTest, IfNDefDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#ifndef") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<IfNDefDirective>(token));
}

TEST(PreprocessorLexerTest, IfNDefDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   ifndef") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<IfNDefDirective>(token));
}

TEST(PreprocessorLexerTest, ElifDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#elif") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<ElifDirective>(token));
}

TEST(PreprocessorLexerTest, ElifDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   elif") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<ElifDirective>(token));
}

TEST(PreprocessorLexerTest, EndIfDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#endif") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<EndIfDirective>(token));
}

TEST(PreprocessorLexerTest, EndIfDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   endif") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<EndIfDirective>(token));
}

TEST(PreprocessorLexerTest, IncludeDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#include") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<IncludeDirective>(token));
}

TEST(PreprocessorLexerTest, IncludeDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   include") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<IncludeDirective>(token));
}

TEST(PreprocessorLexerTest, DefineDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#define") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<DefineDirective>(token));
}

TEST(PreprocessorLexerTest, DefineDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   define") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<DefineDirective>(token));
}

TEST(PreprocessorLexerTest, UnDefDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#undef") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<UnDefDirective>(token));
}

TEST(PreprocessorLexerTest, UnDefDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   undef") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<UnDefDirective>(token));
}

TEST(PreprocessorLexerTest, LineDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#line") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<LineDirective>(token));
}

TEST(PreprocessorLexerTest, LineDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   line") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<LineDirective>(token));
}

TEST(PreprocessorLexerTest, ErrorDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#error") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<ErrorDirective>(token));
}

TEST(PreprocessorLexerTest, ErrorDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   error") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<ErrorDirective>(token));
}

TEST(PreprocessorLexerTest, WarningDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#warning") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<WarningDirective>(token));
}

TEST(PreprocessorLexerTest, WarningDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   warning") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsSyntaxNode<WarningDirective>(token));
}