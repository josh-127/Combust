#include <gtest/gtest.h>
#include "../lexer.hh"

TEST(LexerTest, EmptyFile) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<EofToken>(token));
}

TEST(LexerTest, StrayToken) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "@") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<StrayToken>(token));

    Rc<StrayToken> strayToken{ std::static_pointer_cast<StrayToken>(token) };
    ASSERT_EQ(strayToken->GetOffendingChar(), '@');
}

TEST(LexerTest, IdentifierToken) {
    constexpr const char* name = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    Rc<SourceFile> sourceFile{ CreateSourceFile("", name) };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<IdentifierToken>(token));

    Rc<IdentifierToken> identifierToken{ std::static_pointer_cast<IdentifierToken>(token) };
    ASSERT_EQ(identifierToken->GetName(), name);
}

TEST(LexerTest, ConstKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "const") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<ConstKeyword>(token));
}

TEST(LexerTest, ExternKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "extern") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<ExternKeyword>(token));
}

TEST(LexerTest, StaticKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "static") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<StaticKeyword>(token));
}

TEST(LexerTest, AutoKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "auto") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<AutoKeyword>(token));
}

TEST(LexerTest, VolatileKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "volatile") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<VolatileKeyword>(token));
}

TEST(LexerTest, UnsignedKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "unsigned") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<UnsignedKeyword>(token));
}

TEST(LexerTest, SignedKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "signed") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<SignedKeyword>(token));
}

TEST(LexerTest, VoidKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "void") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<VoidKeyword>(token));
}

TEST(LexerTest, CharKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "char") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<CharKeyword>(token));
}

TEST(LexerTest, ShortKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "short") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<ShortKeyword>(token));
}

TEST(LexerTest, IntKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "int") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<IntKeyword>(token));
}

TEST(LexerTest, LongKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "long") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<LongKeyword>(token));
}

TEST(LexerTest, FloatKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "float") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<FloatKeyword>(token));
}

TEST(LexerTest, DoubleKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "double") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<DoubleKeyword>(token));
}

TEST(LexerTest, EnumKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "enum") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<EnumKeyword>(token));
}

TEST(LexerTest, StructKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "struct") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<StructKeyword>(token));
}

TEST(LexerTest, UnionKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "union") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<UnionKeyword>(token));
}

TEST(LexerTest, TypeDefKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "typedef") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<TypeDefKeyword>(token));
}

TEST(LexerTest, SizeOfKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "sizeof") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<SizeOfKeyword>(token));
}

TEST(LexerTest, RegisterKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "register") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<RegisterKeyword>(token));
}

TEST(LexerTest, GotoKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "goto") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<GotoKeyword>(token));
}

TEST(LexerTest, IfKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "if") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<IfKeyword>(token));
}

TEST(LexerTest, ElseKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "else") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<ElseKeyword>(token));
}

TEST(LexerTest, SwitchKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "switch") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<SwitchKeyword>(token));
}

TEST(LexerTest, CaseKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "case") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<CaseKeyword>(token));
}

TEST(LexerTest, DefaultKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "default") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<DefaultKeyword>(token));
}

TEST(LexerTest, DoKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "do") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<DoKeyword>(token));
}

TEST(LexerTest, WhileKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "while") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<WhileKeyword>(token));
}

TEST(LexerTest, ForKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "for") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<ForKeyword>(token));
}

TEST(LexerTest, BreakKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "break") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<BreakKeyword>(token));
}

TEST(LexerTest, ContinueKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "continue") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<ContinueKeyword>(token));
}

TEST(LexerTest, ReturnKeyword) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "return") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<ReturnKeyword>(token));
}
