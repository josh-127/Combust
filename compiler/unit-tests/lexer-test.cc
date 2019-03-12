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
