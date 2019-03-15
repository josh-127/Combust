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