#include <gtest/gtest.h>
#include "../lexer.hh"

TEST(LexerTest, EmptyFile) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "") };
    Rc<Lexer> lexer{ NewObj<Lexer>(sourceFile) };

    Rc<SyntaxToken> token{ lexer->ReadTokenDirect() };
    ASSERT_TRUE(IsToken<EofToken>(token));
}
