#include <catch.hpp>
#include "../code-lexer.hh"
#include "../source.hh"
#include "../syntax.hh"

TEST_CASE("CodeLexer EmptyFile") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<EofToken>(token));
}

TEST_CASE("CodeLexer StrayToken") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "@") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<StrayToken>(token));

    Rc<StrayToken> strayToken{ As<StrayToken>(token) };
    REQUIRE(strayToken->GetOffendingChar() == '@');
}

TEST_CASE("CodeLexer CommentToken") {
    std::string openingToken{ "/*" };
    std::string closingToken{ "*/" };
    std::string contents{ "The quick brown fox jumps over the lazy dog." };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", openingToken + contents + closingToken) };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<CommentToken>(token));

    Rc<CommentToken> commentToken{ As<CommentToken>(token) };
    REQUIRE(commentToken->GetContents() == contents);
    REQUIRE(commentToken->GetOpeningToken() == openingToken);
    REQUIRE(commentToken->GetClosingToken() == closingToken);
}

TEST_CASE("CodeLexer CommentToken_MissingClosingToken") {
    // NOTE: SourceFile always add a new-line character at the end Contents string.
    std::string openingToken{ "/*" };
    std::string closingToken{ "" };
    std::string contents{ "The quick brown fox jumps over the lazy dog." };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", openingToken + contents + closingToken) };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<CommentToken>(token));

    Rc<CommentToken> commentToken{ As<CommentToken>(token) };
    REQUIRE(commentToken->GetContents() == contents + '\n');
    REQUIRE(commentToken->GetOpeningToken() == openingToken);
    REQUIRE(commentToken->GetClosingToken() == closingToken);
}

TEST_CASE("CodeLexer NumericLiteralToken_Base10_Integer") {
    std::string value{ "1234567890" };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", value) };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<NumericLiteralToken>(token));

    Rc<NumericLiteralToken> literalToken{ As<NumericLiteralToken>(token) };
    REQUIRE(literalToken->GetWholeValue() == value);
    REQUIRE(literalToken->GetFractionalValue() == "");
    REQUIRE(literalToken->GetDotSymbol() == "");
    REQUIRE(literalToken->GetPrefix() == "");
    REQUIRE(literalToken->GetSuffix() == "");
}

TEST_CASE("CodeLexer NumericLiteralToken_Base10_Integer_WithSuffix_L") {
    std::string value{ "1234567890" };
    std::string suffix{ "L" };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", value + suffix) };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<NumericLiteralToken>(token));

    Rc<NumericLiteralToken> literalToken{ As<NumericLiteralToken>(token) };
    REQUIRE(literalToken->GetWholeValue() == value);
    REQUIRE(literalToken->GetFractionalValue() == "");
    REQUIRE(literalToken->GetDotSymbol() == "");
    REQUIRE(literalToken->GetPrefix() == "");
    REQUIRE(literalToken->GetSuffix() == suffix);
}

TEST_CASE("CodeLexer NumericLiteralToken_Base10_Integer_WithInvalidSuffix") {
    std::string value{ "1234567890" };
    std::string suffix{ "FooBar" };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", value + suffix) };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<NumericLiteralToken>(token));

    Rc<NumericLiteralToken> literalToken{ As<NumericLiteralToken>(token) };
    REQUIRE(literalToken->GetWholeValue() == value);
    REQUIRE(literalToken->GetFractionalValue() == "");
    REQUIRE(literalToken->GetDotSymbol() == "");
    REQUIRE(literalToken->GetPrefix() == "");
    REQUIRE(literalToken->GetSuffix() == suffix);
}

TEST_CASE("CodeLexer StringLiteralToken_SingleQuotes_SingleCharacter") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "'A'") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ As<StringLiteralToken>(token) };
    REQUIRE(literalToken->GetValue() == "A");
    REQUIRE(literalToken->GetOpeningQuote() == '\'');
    REQUIRE(literalToken->GetClosingQuote() == '\'');
}

TEST_CASE("CodeLexer StringLiteralToken_SingleQuotes_EscapeSequence") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "'\\n'") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ As<StringLiteralToken>(token) };
    REQUIRE(literalToken->GetValue() == "\\n");
    REQUIRE(literalToken->GetOpeningQuote() == '\'');
    REQUIRE(literalToken->GetClosingQuote() == '\'');
}

TEST_CASE("CodeLexer StringLiteralToken_SingleQuotes_MultipleCharacters") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "'FooBar'") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ As<StringLiteralToken>(token) };
    REQUIRE(literalToken->GetValue() == "FooBar");
    REQUIRE(literalToken->GetOpeningQuote() == '\'');
    REQUIRE(literalToken->GetClosingQuote() == '\'');
}

TEST_CASE("CodeLexer StringLiteralToken_SingleQuotes_MissingClosingQuote") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "'FooBar\n") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ As<StringLiteralToken>(token) };
    REQUIRE(literalToken->GetValue() == "FooBar");
    REQUIRE(literalToken->GetOpeningQuote() == '\'');
    REQUIRE(literalToken->GetClosingQuote() == '\n');
}

TEST_CASE("CodeLexer StringLiteralToken_DoubleQuotes") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "\"FooBar\"") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ As<StringLiteralToken>(token) };
    REQUIRE(literalToken->GetValue() == "FooBar");
    REQUIRE(literalToken->GetOpeningQuote() == '"');
    REQUIRE(literalToken->GetClosingQuote() == '"');
}

TEST_CASE("CodeLexer StringLiteralToken_DoubleQuotes_WithEscapeSequence") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "\"\\n\\r\\t\\0\"") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ As<StringLiteralToken>(token) };
    REQUIRE(literalToken->GetValue() == "\\n\\r\\t\\0");
    REQUIRE(literalToken->GetOpeningQuote() == '"');
    REQUIRE(literalToken->GetClosingQuote() == '"');
}

TEST_CASE("CodeLexer StringLiteralToken_DoubleQuotes_MissingClosingQuote") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "\"FooBar\n") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ As<StringLiteralToken>(token) };
    REQUIRE(literalToken->GetValue() == "FooBar");
    REQUIRE(literalToken->GetOpeningQuote() == '"');
    REQUIRE(literalToken->GetClosingQuote() == '\n');
}

TEST_CASE("CodeLexer IdentifierToken") {
    constexpr const char* name = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    Rc<SourceFile> sourceFile{ CreateSourceFile("", name) };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<IdentifierToken>(token));

    Rc<IdentifierToken> identifierToken{ As<IdentifierToken>(token) };
    REQUIRE(identifierToken->GetName() == name);
}

TEST_CASE("CodeLexer ConstKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "const") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<ConstKeyword>(token));
}

TEST_CASE("CodeLexer ExternKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "extern") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<ExternKeyword>(token));
}

TEST_CASE("CodeLexer StaticKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "static") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<StaticKeyword>(token));
}

TEST_CASE("CodeLexer AutoKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "auto") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<AutoKeyword>(token));
}

TEST_CASE("CodeLexer VolatileKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "volatile") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<VolatileKeyword>(token));
}

TEST_CASE("CodeLexer UnsignedKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "unsigned") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<UnsignedKeyword>(token));
}

TEST_CASE("CodeLexer SignedKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "signed") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<SignedKeyword>(token));
}

TEST_CASE("CodeLexer VoidKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "void") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<VoidKeyword>(token));
}

TEST_CASE("CodeLexer CharKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "char") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<CharKeyword>(token));
}

TEST_CASE("CodeLexer ShortKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "short") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<ShortKeyword>(token));
}

TEST_CASE("CodeLexer IntKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "int") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<IntKeyword>(token));
}

TEST_CASE("CodeLexer LongKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "long") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<LongKeyword>(token));
}

TEST_CASE("CodeLexer FloatKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "float") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<FloatKeyword>(token));
}

TEST_CASE("CodeLexer DoubleKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "double") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<DoubleKeyword>(token));
}

TEST_CASE("CodeLexer EnumKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "enum") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<EnumKeyword>(token));
}

TEST_CASE("CodeLexer StructKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "struct") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<StructKeyword>(token));
}

TEST_CASE("CodeLexer UnionKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "union") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<UnionKeyword>(token));
}

TEST_CASE("CodeLexer TypeDefKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "typedef") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<TypeDefKeyword>(token));
}

TEST_CASE("CodeLexer SizeOfKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "sizeof") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<SizeOfKeyword>(token));
}

TEST_CASE("CodeLexer RegisterKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "register") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<RegisterKeyword>(token));
}

TEST_CASE("CodeLexer GotoKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "goto") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<GotoKeyword>(token));
}

TEST_CASE("CodeLexer IfKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "if") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<IfKeyword>(token));
}

TEST_CASE("CodeLexer ElseKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "else") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<ElseKeyword>(token));
}

TEST_CASE("CodeLexer SwitchKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "switch") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<SwitchKeyword>(token));
}

TEST_CASE("CodeLexer CaseKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "case") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<CaseKeyword>(token));
}

TEST_CASE("CodeLexer DefaultKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "default") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<DefaultKeyword>(token));
}

TEST_CASE("CodeLexer DoKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "do") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<DoKeyword>(token));
}

TEST_CASE("CodeLexer WhileKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "while") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<WhileKeyword>(token));
}

TEST_CASE("CodeLexer ForKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "for") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<ForKeyword>(token));
}

TEST_CASE("CodeLexer BreakKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "break") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<BreakKeyword>(token));
}

TEST_CASE("CodeLexer ContinueKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "continue") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<ContinueKeyword>(token));
}

TEST_CASE("CodeLexer ReturnKeyword") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "return") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<ReturnKeyword>(token));
}

TEST_CASE("CodeLexer LParenSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "(") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<LParenSymbol>(token));
}

TEST_CASE("CodeLexer RParenSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ")") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<RParenSymbol>(token));
}

TEST_CASE("CodeLexer LBracketSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "[") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<LBracketSymbol>(token));
}

TEST_CASE("CodeLexer RBracketSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "]") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<RBracketSymbol>(token));
}

TEST_CASE("CodeLexer LBraceSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "{") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<LBraceSymbol>(token));
}

TEST_CASE("CodeLexer RBraceSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "}") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<RBraceSymbol>(token));
}

TEST_CASE("CodeLexer SemicolonSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ";") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<SemicolonSymbol>(token));
}

TEST_CASE("CodeLexer DotSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ".") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<DotSymbol>(token));
}

TEST_CASE("CodeLexer CommaSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ",") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<CommaSymbol>(token));
}

TEST_CASE("CodeLexer TildeSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "~") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<TildeSymbol>(token));
}

TEST_CASE("CodeLexer QuestionSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "?") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<QuestionSymbol>(token));
}

TEST_CASE("CodeLexer ColonSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ":") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<ColonSymbol>(token));
}

TEST_CASE("CodeLexer PlusSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "+") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<PlusSymbol>(token));
}

TEST_CASE("CodeLexer PlusEqualsSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "+=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<PlusEqualsSymbol>(token));
}

TEST_CASE("CodeLexer PlusPlusSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "++") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<PlusPlusSymbol>(token));
}

TEST_CASE("CodeLexer MinusSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "-") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<MinusSymbol>(token));
}

TEST_CASE("CodeLexer MinusEqualsSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "-=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<MinusEqualsSymbol>(token));
}

TEST_CASE("CodeLexer MinusMinusSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "--") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<MinusMinusSymbol>(token));
}

TEST_CASE("CodeLexer MinusGtSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "->") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<MinusGtSymbol>(token));
}

TEST_CASE("CodeLexer AsteriskSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "*") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<AsteriskSymbol>(token));
}

TEST_CASE("CodeLexer AsteriskEqualsSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "*=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<AsteriskEqualsSymbol>(token));
}

TEST_CASE("CodeLexer SlashSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "/") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<SlashSymbol>(token));
}

TEST_CASE("CodeLexer SlashEqualsSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "/=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<SlashEqualsSymbol>(token));
}

TEST_CASE("CodeLexer PercentSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "%") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<PercentSymbol>(token));
}

TEST_CASE("CodeLexer PercentEqualsSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "%=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<PercentEqualsSymbol>(token));
}

TEST_CASE("CodeLexer LtSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "<") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<LtSymbol>(token));
}

TEST_CASE("CodeLexer LtEqualsSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "<=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<LtEqualsSymbol>(token));
}

TEST_CASE("CodeLexer LtLtSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "<<") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<LtLtSymbol>(token));
}

TEST_CASE("CodeLexer LtLtEqualsSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "<<=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<LtLtEqualsSymbol>(token));
}

TEST_CASE("CodeLexer GtSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ">") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<GtSymbol>(token));
}

TEST_CASE("CodeLexer GtEqualsSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ">=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<GtEqualsSymbol>(token));
}

TEST_CASE("CodeLexer GtGtSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ">>") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<GtGtSymbol>(token));
}

TEST_CASE("CodeLexer GtGtEqualsSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ">>=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<GtGtEqualsSymbol>(token));
}

TEST_CASE("CodeLexer EqualsSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<EqualsSymbol>(token));
}

TEST_CASE("CodeLexer EqualsEqualsSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "==") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<EqualsEqualsSymbol>(token));
}

TEST_CASE("CodeLexer ExclamationSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "!") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<ExclamationSymbol>(token));
}

TEST_CASE("CodeLexer ExclamationEqualsSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "!=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<ExclamationEqualsSymbol>(token));
}

TEST_CASE("CodeLexer AmpersandSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "&") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<AmpersandSymbol>(token));
}

TEST_CASE("CodeLexer AmpersandEqualsSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "&=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<AmpersandEqualsSymbol>(token));
}

TEST_CASE("CodeLexer AmpersandAmpersandSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "&&") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<AmpersandAmpersandSymbol>(token));
}

TEST_CASE("CodeLexer CaretSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "^") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<CaretSymbol>(token));
}

TEST_CASE("CodeLexer CaretEqualsSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "^=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<CaretEqualsSymbol>(token));
}

TEST_CASE("CodeLexer PipeSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "|") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<PipeSymbol>(token));
}

TEST_CASE("CodeLexer PipeEqualsSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "|=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<PipeEqualsSymbol>(token));
}

TEST_CASE("CodeLexer PipePipeSymbol") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "||") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    REQUIRE(IsSyntaxNode<PipePipeSymbol>(token));
}
