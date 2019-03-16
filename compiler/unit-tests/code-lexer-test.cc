#include <gtest/gtest.h>
#include "../code-lexer.hh"
#include "../source.hh"
#include "../syntax.hh"

TEST(CodeLexerTest, EmptyFile) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<EofToken>(token));
}

TEST(CodeLexerTest, StrayToken) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "@") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<StrayToken>(token));

    Rc<StrayToken> strayToken{ As<StrayToken>(token) };
    ASSERT_EQ(strayToken->GetOffendingChar(), '@');
}

TEST(CodeLexerTest, CommentToken) {
    std::string openingToken{ "/*" };
    std::string closingToken{ "*/" };
    std::string contents{ "The quick brown fox jumps over the lazy dog." };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", openingToken + contents + closingToken) };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<CommentToken>(token));

    Rc<CommentToken> commentToken{ As<CommentToken>(token) };
    EXPECT_EQ(commentToken->GetContents(), contents);
    EXPECT_EQ(commentToken->GetOpeningToken(), openingToken);
    EXPECT_EQ(commentToken->GetClosingToken(), closingToken);
}

TEST(CodeLexerTest, CommentToken_MissingClosingToken) {
    // NOTE: SourceFile always add a new-line character at the end Contents string.
    std::string openingToken{ "/*" };
    std::string closingToken{ "" };
    std::string contents{ "The quick brown fox jumps over the lazy dog." };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", openingToken + contents + closingToken) };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<CommentToken>(token));

    Rc<CommentToken> commentToken{ As<CommentToken>(token) };
    EXPECT_EQ(commentToken->GetContents(), contents + '\n');
    EXPECT_EQ(commentToken->GetOpeningToken(), openingToken);
    EXPECT_EQ(commentToken->GetClosingToken(), closingToken);
}

TEST(CodeLexerTest, NumericLiteralToken_Base10_Integer) {
    std::string value{ "1234567890" };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", value) };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<NumericLiteralToken>(token));

    Rc<NumericLiteralToken> literalToken{ As<NumericLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetWholeValue(), value);
    EXPECT_EQ(literalToken->GetFractionalValue(), "");
    EXPECT_EQ(literalToken->GetDotSymbol(), "");
    EXPECT_EQ(literalToken->GetPrefix(), "");
    EXPECT_EQ(literalToken->GetSuffix(), "");
}

TEST(CodeLexerTest, NumericLiteralToken_Base10_Integer_WithSuffix_L) {
    std::string value{ "1234567890" };
    std::string suffix{ "L" };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", value + suffix) };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<NumericLiteralToken>(token));

    Rc<NumericLiteralToken> literalToken{ As<NumericLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetWholeValue(), value);
    EXPECT_EQ(literalToken->GetFractionalValue(), "");
    EXPECT_EQ(literalToken->GetDotSymbol(), "");
    EXPECT_EQ(literalToken->GetPrefix(), "");
    EXPECT_EQ(literalToken->GetSuffix(), suffix);
}

TEST(CodeLexerTest, NumericLiteralToken_Base10_Integer_WithInvalidSuffix) {
    std::string value{ "1234567890" };
    std::string suffix{ "FooBar" };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", value + suffix) };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<NumericLiteralToken>(token));

    Rc<NumericLiteralToken> literalToken{ As<NumericLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetWholeValue(), value);
    EXPECT_EQ(literalToken->GetFractionalValue(), "");
    EXPECT_EQ(literalToken->GetDotSymbol(), "");
    EXPECT_EQ(literalToken->GetPrefix(), "");
    EXPECT_EQ(literalToken->GetSuffix(), suffix);
}

TEST(CodeLexerTest, StringLiteralToken_SingleQuotes_SingleCharacter) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "'A'") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ As<StringLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetValue(), "A");
    EXPECT_EQ(literalToken->GetOpeningQuote(), '\'');
    EXPECT_EQ(literalToken->GetClosingQuote(), '\'');
}

TEST(CodeLexerTest, StringLiteralToken_SingleQuotes_EscapeSequence) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "'\\n'") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ As<StringLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetValue(), "\\n");
    EXPECT_EQ(literalToken->GetOpeningQuote(), '\'');
    EXPECT_EQ(literalToken->GetClosingQuote(), '\'');
}

TEST(CodeLexerTest, StringLiteralToken_SingleQuotes_MultipleCharacters) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "'FooBar'") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ As<StringLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetValue(), "FooBar");
    EXPECT_EQ(literalToken->GetOpeningQuote(), '\'');
    EXPECT_EQ(literalToken->GetClosingQuote(), '\'');
}

TEST(CodeLexerTest, StringLiteralToken_SingleQuotes_MissingClosingQuote) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "'FooBar\n") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ As<StringLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetValue(), "FooBar");
    EXPECT_EQ(literalToken->GetOpeningQuote(), '\'');
    EXPECT_EQ(literalToken->GetClosingQuote(), '\n');
}

TEST(CodeLexerTest, StringLiteralToken_DoubleQuotes) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "\"FooBar\"") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ As<StringLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetValue(), "FooBar");
    EXPECT_EQ(literalToken->GetOpeningQuote(), '"');
    EXPECT_EQ(literalToken->GetClosingQuote(), '"');
}

TEST(CodeLexerTest, StringLiteralToken_DoubleQuotes_WithEscapeSequence) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "\"\\n\\r\\t\\0\"") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ As<StringLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetValue(), "\\n\\r\\t\\0");
    EXPECT_EQ(literalToken->GetOpeningQuote(), '"');
    EXPECT_EQ(literalToken->GetClosingQuote(), '"');
}

TEST(CodeLexerTest, StringLiteralToken_DoubleQuotes_MissingClosingQuote) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "\"FooBar\n") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ As<StringLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetValue(), "FooBar");
    EXPECT_EQ(literalToken->GetOpeningQuote(), '"');
    EXPECT_EQ(literalToken->GetClosingQuote(), '\n');
}

TEST(CodeLexerTest, IdentifierToken) {
    constexpr const char* name = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    Rc<SourceFile> sourceFile{ CreateSourceFile("", name) };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(token));

    Rc<IdentifierToken> identifierToken{ As<IdentifierToken>(token) };
    ASSERT_EQ(identifierToken->GetName(), name);
}

TEST(CodeLexerTest, ConstKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "const") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<ConstKeyword>(token));
}

TEST(CodeLexerTest, ExternKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "extern") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<ExternKeyword>(token));
}

TEST(CodeLexerTest, StaticKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "static") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<StaticKeyword>(token));
}

TEST(CodeLexerTest, AutoKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "auto") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<AutoKeyword>(token));
}

TEST(CodeLexerTest, VolatileKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "volatile") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<VolatileKeyword>(token));
}

TEST(CodeLexerTest, UnsignedKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "unsigned") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<UnsignedKeyword>(token));
}

TEST(CodeLexerTest, SignedKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "signed") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<SignedKeyword>(token));
}

TEST(CodeLexerTest, VoidKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "void") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<VoidKeyword>(token));
}

TEST(CodeLexerTest, CharKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "char") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<CharKeyword>(token));
}

TEST(CodeLexerTest, ShortKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "short") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<ShortKeyword>(token));
}

TEST(CodeLexerTest, IntKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "int") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<IntKeyword>(token));
}

TEST(CodeLexerTest, LongKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "long") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<LongKeyword>(token));
}

TEST(CodeLexerTest, FloatKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "float") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<FloatKeyword>(token));
}

TEST(CodeLexerTest, DoubleKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "double") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<DoubleKeyword>(token));
}

TEST(CodeLexerTest, EnumKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "enum") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<EnumKeyword>(token));
}

TEST(CodeLexerTest, StructKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "struct") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<StructKeyword>(token));
}

TEST(CodeLexerTest, UnionKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "union") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<UnionKeyword>(token));
}

TEST(CodeLexerTest, TypeDefKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "typedef") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<TypeDefKeyword>(token));
}

TEST(CodeLexerTest, SizeOfKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "sizeof") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<SizeOfKeyword>(token));
}

TEST(CodeLexerTest, RegisterKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "register") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<RegisterKeyword>(token));
}

TEST(CodeLexerTest, GotoKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "goto") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<GotoKeyword>(token));
}

TEST(CodeLexerTest, IfKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "if") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<IfKeyword>(token));
}

TEST(CodeLexerTest, ElseKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "else") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<ElseKeyword>(token));
}

TEST(CodeLexerTest, SwitchKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "switch") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<SwitchKeyword>(token));
}

TEST(CodeLexerTest, CaseKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "case") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<CaseKeyword>(token));
}

TEST(CodeLexerTest, DefaultKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "default") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<DefaultKeyword>(token));
}

TEST(CodeLexerTest, DoKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "do") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<DoKeyword>(token));
}

TEST(CodeLexerTest, WhileKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "while") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<WhileKeyword>(token));
}

TEST(CodeLexerTest, ForKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "for") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<ForKeyword>(token));
}

TEST(CodeLexerTest, BreakKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "break") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<BreakKeyword>(token));
}

TEST(CodeLexerTest, ContinueKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "continue") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<ContinueKeyword>(token));
}

TEST(CodeLexerTest, ReturnKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "return") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<ReturnKeyword>(token));
}

TEST(CodeLexerTest, LParenSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "(") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<LParenSymbol>(token));
}

TEST(CodeLexerTest, RParenSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ")") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<RParenSymbol>(token));
}

TEST(CodeLexerTest, LBracketSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "[") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<LBracketSymbol>(token));
}

TEST(CodeLexerTest, RBracketSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "]") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<RBracketSymbol>(token));
}

TEST(CodeLexerTest, LBraceSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "{") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<LBraceSymbol>(token));
}

TEST(CodeLexerTest, RBraceSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "}") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<RBraceSymbol>(token));
}

TEST(CodeLexerTest, SemicolonSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ";") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<SemicolonSymbol>(token));
}

TEST(CodeLexerTest, DotSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ".") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<DotSymbol>(token));
}

TEST(CodeLexerTest, CommaSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ",") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<CommaSymbol>(token));
}

TEST(CodeLexerTest, TildeSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "~") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<TildeSymbol>(token));
}

TEST(CodeLexerTest, QuestionSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "?") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<QuestionSymbol>(token));
}

TEST(CodeLexerTest, ColonSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ":") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<ColonSymbol>(token));
}

TEST(CodeLexerTest, PlusSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "+") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<PlusSymbol>(token));
}

TEST(CodeLexerTest, PlusEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "+=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<PlusEqualsSymbol>(token));
}

TEST(CodeLexerTest, PlusPlusSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "++") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<PlusPlusSymbol>(token));
}

TEST(CodeLexerTest, MinusSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "-") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<MinusSymbol>(token));
}

TEST(CodeLexerTest, MinusEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "-=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<MinusEqualsSymbol>(token));
}

TEST(CodeLexerTest, MinusMinusSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "--") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<MinusMinusSymbol>(token));
}

TEST(CodeLexerTest, MinusGtSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "->") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<MinusGtSymbol>(token));
}

TEST(CodeLexerTest, AsteriskSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "*") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<AsteriskSymbol>(token));
}

TEST(CodeLexerTest, AsteriskEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "*=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<AsteriskEqualsSymbol>(token));
}

TEST(CodeLexerTest, SlashSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "/") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<SlashSymbol>(token));
}

TEST(CodeLexerTest, SlashEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "/=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<SlashEqualsSymbol>(token));
}

TEST(CodeLexerTest, PercentSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "%") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<PercentSymbol>(token));
}

TEST(CodeLexerTest, PercentEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "%=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<PercentEqualsSymbol>(token));
}

TEST(CodeLexerTest, LtSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "<") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<LtSymbol>(token));
}

TEST(CodeLexerTest, LtEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "<=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<LtEqualsSymbol>(token));
}

TEST(CodeLexerTest, LtLtSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "<<") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<LtLtSymbol>(token));
}

TEST(CodeLexerTest, LtLtEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "<<=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<LtLtEqualsSymbol>(token));
}

TEST(CodeLexerTest, GtSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ">") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<GtSymbol>(token));
}

TEST(CodeLexerTest, GtEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ">=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<GtEqualsSymbol>(token));
}

TEST(CodeLexerTest, GtGtSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ">>") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<GtGtSymbol>(token));
}

TEST(CodeLexerTest, GtGtEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ">>=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<GtGtEqualsSymbol>(token));
}

TEST(CodeLexerTest, EqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<EqualsSymbol>(token));
}

TEST(CodeLexerTest, EqualsEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "==") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<EqualsEqualsSymbol>(token));
}

TEST(CodeLexerTest, ExclamationSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "!") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<ExclamationSymbol>(token));
}

TEST(CodeLexerTest, ExclamationEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "!=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<ExclamationEqualsSymbol>(token));
}

TEST(CodeLexerTest, AmpersandSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "&") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<AmpersandSymbol>(token));
}

TEST(CodeLexerTest, AmpersandEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "&=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<AmpersandEqualsSymbol>(token));
}

TEST(CodeLexerTest, AmpersandAmpersandSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "&&") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<AmpersandAmpersandSymbol>(token));
}

TEST(CodeLexerTest, CaretSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "^") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<CaretSymbol>(token));
}

TEST(CodeLexerTest, CaretEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "^=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<CaretEqualsSymbol>(token));
}

TEST(CodeLexerTest, PipeSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "|") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<PipeSymbol>(token));
}

TEST(CodeLexerTest, PipeEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "|=") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<PipeEqualsSymbol>(token));
}

TEST(CodeLexerTest, PipePipeSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "||") };
    Rc<CodeLexer> lexer{ NewObj<CodeLexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsSyntaxNode<PipePipeSymbol>(token));
}
