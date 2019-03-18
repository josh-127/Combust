#include <catch.hpp>
#include "../preprocessor-lexer.hh"
#include "../source.hh"
#include "../syntax.hh"

TEST_CASE("PreprocessorLexer EmptyFile") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE(IsSyntaxNode<EofToken>(token));
}

TEST_CASE("PreprocessorLexer HStringLiteralToken") {
    std::string path{ "FooBar" };
    Rc<SourceFile> sourceFile{
        CreateSourceFile("", "#include <" + path + ">")
    };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> firstToken{ preprocessor->ReadToken() };
    REQUIRE((firstToken->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<IncludeDirective>(firstToken));

    Rc<SyntaxToken> secondToken{ preprocessor->ReadToken() };
    REQUIRE((secondToken->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) == 0);
    REQUIRE(IsSyntaxNode<StringLiteralToken>(secondToken));

    Rc<StringLiteralToken> literalToken{ std::static_pointer_cast<StringLiteralToken>(secondToken) };
    REQUIRE(literalToken->GetValue() == path);
    REQUIRE(literalToken->GetOpeningQuote() == '<');
    REQUIRE(literalToken->GetClosingQuote() == '>');
}

TEST_CASE("PreprocessorLexer HStringLiteralToken_WithoutWhitespaceInBetween") {
    std::string path{ "FooBar" };
    Rc<SourceFile> sourceFile{
        CreateSourceFile("", "#include<" + path + ">")
    };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> firstToken{ preprocessor->ReadToken() };
    REQUIRE((firstToken->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<IncludeDirective>(firstToken));

    Rc<SyntaxToken> secondToken{ preprocessor->ReadToken() };
    REQUIRE((secondToken->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) == 0);
    REQUIRE(IsSyntaxNode<StringLiteralToken>(secondToken));

    Rc<StringLiteralToken> literalToken{ std::static_pointer_cast<StringLiteralToken>(secondToken) };
    REQUIRE(literalToken->GetValue() == path);
    REQUIRE(literalToken->GetOpeningQuote() == '<');
    REQUIRE(literalToken->GetClosingQuote() == '>');
}

TEST_CASE("PreprocessorLexer IdentifierToken") {
    Rc<SourceFile> sourceFile{
        CreateSourceFile(
            "",
            "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
        )
    };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE(IsSyntaxNode<IdentifierToken>(token));
}

TEST_CASE("PreprocessorLexer InvalidDirective") {
    std::string name{ "FooBar" };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", '#' + name) };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE(IsSyntaxNode<InvalidDirective>(token));

    Rc<InvalidDirective> directive{ std::static_pointer_cast<InvalidDirective>(token) };
    REQUIRE(directive->GetName() == name);
    REQUIRE((directive->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
}

TEST_CASE("PreprocessorLexer InvalidDirective_WithWhitespaceInBetween") {
    std::string name{ "FooBar" };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   " + name) };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<InvalidDirective>(token));

    Rc<InvalidDirective> directive{ std::static_pointer_cast<InvalidDirective>(token) };
    REQUIRE(directive->GetName() == name);
}

TEST_CASE("PreprocessorLexer IfDirective") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#if") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<IfDirective>(token));
}

TEST_CASE("PreprocessorLexer IfDirective_WithWhitespaceInBetween") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   if") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<IfDirective>(token));
}

TEST_CASE("PreprocessorLexer IfDefDirective") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#ifdef") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<IfDefDirective>(token));
}

TEST_CASE("PreprocessorLexer IfDefDirective_WithWhitespaceInBetween") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   ifdef") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<IfDefDirective>(token));
}

TEST_CASE("PreprocessorLexer IfNDefDirective") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#ifndef") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<IfNDefDirective>(token));
}

TEST_CASE("PreprocessorLexer IfNDefDirective_WithWhitespaceInBetween") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   ifndef") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<IfNDefDirective>(token));
}

TEST_CASE("PreprocessorLexer ElifDirective") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#elif") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<ElifDirective>(token));
}

TEST_CASE("PreprocessorLexer ElifDirective_WithWhitespaceInBetween") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   elif") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<ElifDirective>(token));
}

TEST_CASE("PreprocessorLexer EndIfDirective") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#endif") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<EndIfDirective>(token));
}

TEST_CASE("PreprocessorLexer EndIfDirective_WithWhitespaceInBetween") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   endif") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<EndIfDirective>(token));
}

TEST_CASE("PreprocessorLexer IncludeDirective") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#include") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<IncludeDirective>(token));
}

TEST_CASE("PreprocessorLexer IncludeDirective_WithWhitespaceInBetween") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   include") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<IncludeDirective>(token));
}

TEST_CASE("PreprocessorLexer DefineDirective") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#define") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<DefineDirective>(token));
}

TEST_CASE("PreprocessorLexer DefineDirective_WithWhitespaceInBetween") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   define") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<DefineDirective>(token));
}

TEST_CASE("PreprocessorLexer UnDefDirective") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#undef") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<UnDefDirective>(token));
}

TEST_CASE("PreprocessorLexer UnDefDirective_WithWhitespaceInBetween") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   undef") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<UnDefDirective>(token));
}

TEST_CASE("PreprocessorLexer LineDirective") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#line") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<LineDirective>(token));
}

TEST_CASE("PreprocessorLexer LineDirective_WithWhitespaceInBetween") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   line") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<LineDirective>(token));
}

TEST_CASE("PreprocessorLexer ErrorDirective") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#error") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<ErrorDirective>(token));
}

TEST_CASE("PreprocessorLexer ErrorDirective_WithWhitespaceInBetween") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   error") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<ErrorDirective>(token));
}

TEST_CASE("PreprocessorLexer WarningDirective") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#warning") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<WarningDirective>(token));
}

TEST_CASE("PreprocessorLexer WarningDirective_WithWhitespaceInBetween") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "#   warning") };
    Rc<PreprocessorLexer> preprocessor{ NewObj<PreprocessorLexer>(sourceFile) };

    Rc<SyntaxToken> token{ preprocessor->ReadToken() };
    REQUIRE((token->GetFlags() & SyntaxToken::BEGINNING_OF_LINE) != 0);
    REQUIRE(IsSyntaxNode<WarningDirective>(token));
}
