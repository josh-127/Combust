#include <gtest/gtest.h>
#include "../lexer.hh"

TEST(LexerTest, EmptyFile) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<EofToken>(token));
}

TEST(LexerTest, StrayToken) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "@") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<StrayToken>(token));

    Rc<StrayToken> strayToken{ std::static_pointer_cast<StrayToken>(token) };
    ASSERT_EQ(strayToken->GetOffendingChar(), '@');
}

TEST(LexerTest, CommentToken) {
    std::string openingToken{ "/*" };
    std::string closingToken{ "*/" };
    std::string contents{ "The quick brown fox jumps over the lazy dog." };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", openingToken + contents + closingToken) };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<CommentToken>(token));

    Rc<CommentToken> commentToken{ std::static_pointer_cast<CommentToken>(token) };
    EXPECT_EQ(commentToken->GetContents(), contents);
    EXPECT_EQ(commentToken->GetOpeningToken(), openingToken);
    EXPECT_EQ(commentToken->GetClosingToken(), closingToken);
}

TEST(LexerTest, CommentToken_MissingClosingToken) {
    // NOTE: SourceFile always add a new-line character at the end Contents string.
    std::string openingToken{ "/*" };
    std::string closingToken{ "" };
    std::string contents{ "The quick brown fox jumps over the lazy dog." };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", openingToken + contents + closingToken) };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<CommentToken>(token));

    Rc<CommentToken> commentToken{ std::static_pointer_cast<CommentToken>(token) };
    EXPECT_EQ(commentToken->GetContents(), contents + '\n');
    EXPECT_EQ(commentToken->GetOpeningToken(), openingToken);
    EXPECT_EQ(commentToken->GetClosingToken(), closingToken);
}

TEST(LexerTest, NumericLiteralToken_Base10_Integer) {
    std::string value{ "1234567890" };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", value) };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<NumericLiteralToken>(token));

    Rc<NumericLiteralToken> literalToken{ std::static_pointer_cast<NumericLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetWholeValue(), value);
    EXPECT_EQ(literalToken->GetFractionalValue(), "");
    EXPECT_EQ(literalToken->GetDotSymbol(), "");
    EXPECT_EQ(literalToken->GetPrefix(), "");
    EXPECT_EQ(literalToken->GetSuffix(), "");
}

TEST(LexerTest, NumericLiteralToken_Base10_Integer_WithSuffix_L) {
    std::string value{ "1234567890" };
    std::string suffix{ "L" };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", value + suffix) };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<NumericLiteralToken>(token));

    Rc<NumericLiteralToken> literalToken{ std::static_pointer_cast<NumericLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetWholeValue(), value);
    EXPECT_EQ(literalToken->GetFractionalValue(), "");
    EXPECT_EQ(literalToken->GetDotSymbol(), "");
    EXPECT_EQ(literalToken->GetPrefix(), "");
    EXPECT_EQ(literalToken->GetSuffix(), suffix);
}

TEST(LexerTest, NumericLiteralToken_Base10_Integer_WithInvalidSuffix) {
    std::string value{ "1234567890" };
    std::string suffix{ "FooBar" };
    Rc<SourceFile> sourceFile{ CreateSourceFile("", value + suffix) };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<NumericLiteralToken>(token));

    Rc<NumericLiteralToken> literalToken{ std::static_pointer_cast<NumericLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetWholeValue(), value);
    EXPECT_EQ(literalToken->GetFractionalValue(), "");
    EXPECT_EQ(literalToken->GetDotSymbol(), "");
    EXPECT_EQ(literalToken->GetPrefix(), "");
    EXPECT_EQ(literalToken->GetSuffix(), suffix);
}

TEST(LexerTest, StringLiteralToken_SingleQuotes_SingleCharacter) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "'A'") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ std::static_pointer_cast<StringLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetValue(), "A");
    EXPECT_EQ(literalToken->GetOpeningQuote(), '\'');
    EXPECT_EQ(literalToken->GetClosingQuote(), '\'');
}

TEST(LexerTest, StringLiteralToken_SingleQuotes_EscapeSequence) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "'\\n'") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ std::static_pointer_cast<StringLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetValue(), "\\n");
    EXPECT_EQ(literalToken->GetOpeningQuote(), '\'');
    EXPECT_EQ(literalToken->GetClosingQuote(), '\'');
}

TEST(LexerTest, StringLiteralToken_SingleQuotes_MultipleCharacters) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "'FooBar'") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ std::static_pointer_cast<StringLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetValue(), "FooBar");
    EXPECT_EQ(literalToken->GetOpeningQuote(), '\'');
    EXPECT_EQ(literalToken->GetClosingQuote(), '\'');
}

TEST(LexerTest, StringLiteralToken_SingleQuotes_MissingClosingQuote) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "'FooBar\n") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ std::static_pointer_cast<StringLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetValue(), "FooBar");
    EXPECT_EQ(literalToken->GetOpeningQuote(), '\'');
    EXPECT_EQ(literalToken->GetClosingQuote(), '\n');
}

TEST(LexerTest, StringLiteralToken_DoubleQuotes) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "\"FooBar\"") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ std::static_pointer_cast<StringLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetValue(), "FooBar");
    EXPECT_EQ(literalToken->GetOpeningQuote(), '"');
    EXPECT_EQ(literalToken->GetClosingQuote(), '"');
}

TEST(LexerTest, StringLiteralToken_DoubleQuotes_WithEscapeSequence) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "\"\\n\\r\\t\\0\"") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ std::static_pointer_cast<StringLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetValue(), "\\n\\r\\t\\0");
    EXPECT_EQ(literalToken->GetOpeningQuote(), '"');
    EXPECT_EQ(literalToken->GetClosingQuote(), '"');
}

TEST(LexerTest, StringLiteralToken_DoubleQuotes_MissingClosingQuote) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "\"FooBar\n") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<StringLiteralToken>(token));

    Rc<StringLiteralToken> literalToken{ std::static_pointer_cast<StringLiteralToken>(token) };
    EXPECT_EQ(literalToken->GetValue(), "FooBar");
    EXPECT_EQ(literalToken->GetOpeningQuote(), '"');
    EXPECT_EQ(literalToken->GetClosingQuote(), '\n');
}

TEST(LexerTest, IdentifierToken) {
    constexpr const char* name = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    Rc<SourceFile> sourceFile{ CreateSourceFile("", name) };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<IdentifierToken>(token));

    Rc<IdentifierToken> identifierToken{ std::static_pointer_cast<IdentifierToken>(token) };
    ASSERT_EQ(identifierToken->GetName(), name);
}

TEST(LexerTest, ConstKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "const") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<ConstKeyword>(token));
}

TEST(LexerTest, ExternKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "extern") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<ExternKeyword>(token));
}

TEST(LexerTest, StaticKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "static") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<StaticKeyword>(token));
}

TEST(LexerTest, AutoKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "auto") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<AutoKeyword>(token));
}

TEST(LexerTest, VolatileKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "volatile") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<VolatileKeyword>(token));
}

TEST(LexerTest, UnsignedKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "unsigned") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<UnsignedKeyword>(token));
}

TEST(LexerTest, SignedKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "signed") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<SignedKeyword>(token));
}

TEST(LexerTest, VoidKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "void") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<VoidKeyword>(token));
}

TEST(LexerTest, CharKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "char") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<CharKeyword>(token));
}

TEST(LexerTest, ShortKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "short") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<ShortKeyword>(token));
}

TEST(LexerTest, IntKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "int") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<IntKeyword>(token));
}

TEST(LexerTest, LongKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "long") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<LongKeyword>(token));
}

TEST(LexerTest, FloatKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "float") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<FloatKeyword>(token));
}

TEST(LexerTest, DoubleKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "double") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<DoubleKeyword>(token));
}

TEST(LexerTest, EnumKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "enum") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<EnumKeyword>(token));
}

TEST(LexerTest, StructKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "struct") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<StructKeyword>(token));
}

TEST(LexerTest, UnionKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "union") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<UnionKeyword>(token));
}

TEST(LexerTest, TypeDefKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "typedef") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<TypeDefKeyword>(token));
}

TEST(LexerTest, SizeOfKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "sizeof") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<SizeOfKeyword>(token));
}

TEST(LexerTest, RegisterKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "register") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<RegisterKeyword>(token));
}

TEST(LexerTest, GotoKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "goto") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<GotoKeyword>(token));
}

TEST(LexerTest, IfKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "if") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<IfKeyword>(token));
}

TEST(LexerTest, ElseKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "else") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<ElseKeyword>(token));
}

TEST(LexerTest, SwitchKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "switch") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<SwitchKeyword>(token));
}

TEST(LexerTest, CaseKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "case") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<CaseKeyword>(token));
}

TEST(LexerTest, DefaultKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "default") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<DefaultKeyword>(token));
}

TEST(LexerTest, DoKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "do") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<DoKeyword>(token));
}

TEST(LexerTest, WhileKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "while") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<WhileKeyword>(token));
}

TEST(LexerTest, ForKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "for") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<ForKeyword>(token));
}

TEST(LexerTest, BreakKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "break") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<BreakKeyword>(token));
}

TEST(LexerTest, ContinueKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "continue") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<ContinueKeyword>(token));
}

TEST(LexerTest, ReturnKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "return") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<ReturnKeyword>(token));
}

TEST(LexerTest, LParenSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "(") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<LParenSymbol>(token));
}

TEST(LexerTest, RParenSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ")") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<RParenSymbol>(token));
}

TEST(LexerTest, LBracketSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "[") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<LBracketSymbol>(token));
}

TEST(LexerTest, RBracketSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "]") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<RBracketSymbol>(token));
}

TEST(LexerTest, LBraceSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "{") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<LBraceSymbol>(token));
}

TEST(LexerTest, RBraceSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "}") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<RBraceSymbol>(token));
}

TEST(LexerTest, SemicolonSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ";") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<SemicolonSymbol>(token));
}

TEST(LexerTest, DotSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ".") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<DotSymbol>(token));
}

TEST(LexerTest, CommaSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ",") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<CommaSymbol>(token));
}

TEST(LexerTest, TildeSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "~") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<TildeSymbol>(token));
}

TEST(LexerTest, QuestionSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "?") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<QuestionSymbol>(token));
}

TEST(LexerTest, ColonSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ":") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<ColonSymbol>(token));
}

TEST(LexerTest, PlusSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "+") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<PlusSymbol>(token));
}

TEST(LexerTest, PlusEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "+=") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<PlusEqualsSymbol>(token));
}

TEST(LexerTest, PlusPlusSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "++") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<PlusPlusSymbol>(token));
}

TEST(LexerTest, MinusSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "-") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<MinusSymbol>(token));
}

TEST(LexerTest, MinusEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "-=") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<MinusEqualsSymbol>(token));
}

TEST(LexerTest, MinusMinusSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "--") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<MinusMinusSymbol>(token));
}

TEST(LexerTest, MinusGtSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "->") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<MinusGtSymbol>(token));
}

TEST(LexerTest, AsteriskSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "*") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<AsteriskSymbol>(token));
}

TEST(LexerTest, AsteriskEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "*=") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<AsteriskEqualsSymbol>(token));
}

TEST(LexerTest, SlashSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "/") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<SlashSymbol>(token));
}

TEST(LexerTest, SlashEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "/=") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<SlashEqualsSymbol>(token));
}

TEST(LexerTest, PercentSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "%") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<PercentSymbol>(token));
}

TEST(LexerTest, PercentEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "%=") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<PercentEqualsSymbol>(token));
}

TEST(LexerTest, LtSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "<") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<LtSymbol>(token));
}

TEST(LexerTest, LtEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "<=") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<LtEqualsSymbol>(token));
}

TEST(LexerTest, LtLtSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "<<") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<LtLtSymbol>(token));
}

TEST(LexerTest, LtLtEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "<<=") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<LtLtEqualsSymbol>(token));
}

TEST(LexerTest, GtSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ">") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<GtSymbol>(token));
}

TEST(LexerTest, GtEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ">=") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<GtEqualsSymbol>(token));
}

TEST(LexerTest, GtGtSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ">>") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<GtGtSymbol>(token));
}

TEST(LexerTest, GtGtEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", ">>=") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<GtGtEqualsSymbol>(token));
}

TEST(LexerTest, EqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "=") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<EqualsSymbol>(token));
}

TEST(LexerTest, EqualsEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "==") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<EqualsEqualsSymbol>(token));
}

TEST(LexerTest, ExclamationSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "!") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<ExclamationSymbol>(token));
}

TEST(LexerTest, ExclamationEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "!=") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<ExclamationEqualsSymbol>(token));
}

TEST(LexerTest, AmpersandSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "&") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<AmpersandSymbol>(token));
}

TEST(LexerTest, AmpersandEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "&=") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<AmpersandEqualsSymbol>(token));
}

TEST(LexerTest, AmpersandAmpersandSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "&&") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<AmpersandAmpersandSymbol>(token));
}

TEST(LexerTest, CaretSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "^") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<CaretSymbol>(token));
}

TEST(LexerTest, CaretEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "^=") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<CaretEqualsSymbol>(token));
}

TEST(LexerTest, PipeSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "|") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<PipeSymbol>(token));
}

TEST(LexerTest, PipeEqualsSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "|=") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<PipeEqualsSymbol>(token));
}

TEST(LexerTest, PipePipeSymbol) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "||") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadToken() };
    ASSERT_TRUE(IsToken<PipePipeSymbol>(token));
}
