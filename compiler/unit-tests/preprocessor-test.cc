#include <gtest/gtest.h>
#include "../preprocessor.hh"
#include "../source.hh"
#include "../syntax.hh"

TEST(PreprocessorTest, EmptyFile) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    ASSERT_TRUE(IsToken<EofToken>(token));
}

TEST(PreprocessorTest, IdentifierToken) {
    Rc<SourceFile> sourceFile{
        CreateSourceFile(
            "",
            "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
        )
    };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    ASSERT_TRUE(IsToken<IdentifierToken>(token));
}

TEST(PreprocessorTest, InvalidDirective) {
    std::string name{ "FooBar" };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", '#' + name) };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    ASSERT_TRUE(IsToken<InvalidDirective>(token));

    Rc<InvalidDirective> directive{ std::static_pointer_cast<InvalidDirective>(token) };
    EXPECT_EQ(directive->GetName(), name);
    EXPECT_TRUE(directive->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
}

TEST(PreprocessorTest, InvalidDirective_WithWhitespaceInBetween) {
    std::string name{ "FooBar" };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   " + name) };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<InvalidDirective>(token));

    Rc<InvalidDirective> directive{ std::static_pointer_cast<InvalidDirective>(token) };
    EXPECT_EQ(directive->GetName(), name);
}

TEST(PreprocessorTest, IfDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#if") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<IfDirective>(token));
}

TEST(PreprocessorTest, IfDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   if") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<IfDirective>(token));
}

TEST(PreprocessorTest, IfDefDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#ifdef") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<IfDefDirective>(token));
}

TEST(PreprocessorTest, IfDefDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   ifdef") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<IfDefDirective>(token));
}

TEST(PreprocessorTest, IfNDefDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#ifndef") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<IfNDefDirective>(token));
}

TEST(PreprocessorTest, IfNDefDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   ifndef") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<IfNDefDirective>(token));
}

TEST(PreprocessorTest, ElifDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#elif") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<ElifDirective>(token));
}

TEST(PreprocessorTest, ElifDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   elif") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<ElifDirective>(token));
}

TEST(PreprocessorTest, EndIfDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#endif") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<EndIfDirective>(token));
}

TEST(PreprocessorTest, EndIfDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   endif") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<EndIfDirective>(token));
}

TEST(PreprocessorTest, IncludeDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#include") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<IncludeDirective>(token));
}

TEST(PreprocessorTest, IncludeDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   include") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<IncludeDirective>(token));
}

TEST(PreprocessorTest, DefineDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#define") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<DefineDirective>(token));
}

TEST(PreprocessorTest, DefineDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   define") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<DefineDirective>(token));
}

TEST(PreprocessorTest, UnDefDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#undef") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<UnDefDirective>(token));
}

TEST(PreprocessorTest, UnDefDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   undef") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<UnDefDirective>(token));
}

TEST(PreprocessorTest, LineDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#line") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<LineDirective>(token));
}

TEST(PreprocessorTest, LineDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   line") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<LineDirective>(token));
}

TEST(PreprocessorTest, ErrorDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#error") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<ErrorDirective>(token));
}

TEST(PreprocessorTest, ErrorDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   error") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<ErrorDirective>(token));
}

TEST(PreprocessorTest, WarningDirective) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#warning") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<WarningDirective>(token));
}

TEST(PreprocessorTest, WarningDirective_WithWhitespaceInBetween) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   warning") };
    Rc<Preprocessor> preprocessor{ NewObj<Preprocessor>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    EXPECT_TRUE(token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE);
    ASSERT_TRUE(IsToken<WarningDirective>(token));
}