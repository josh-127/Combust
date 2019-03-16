#include <gtest/gtest.h>
#include "../backtracking-lexer.hh"
#include "../language-parser.hh"
#include "../lexer.hh"
#include "../syntax.hh"
#include <vector>

class MockLexer : public Object, public virtual ILexer {
public:
    explicit MockLexer(const std::vector<Rc<SyntaxToken>>& tokens);
    virtual ~MockLexer();

    Rc<SyntaxToken> ReadToken() override;

private:
    std::vector<Rc<SyntaxToken>> tokens{ };
    size_t currentPos{ 0 };
};

MockLexer::MockLexer(const std::vector<Rc<SyntaxToken>>& tokens) :
    tokens{ tokens },
    currentPos{ 0 }
{}

MockLexer::~MockLexer() {}

Rc<SyntaxToken> MockLexer::ReadToken() {
    Rc<SyntaxToken> token{ tokens[currentPos] };
    if (currentPos + 1 < tokens.size())
        ++currentPos;
    return token;
}

static Rc<EofToken> NewEofToken() {
    return NewObj<EofToken>();
}

static Rc<IdentifierToken> NewIdentifierToken(const std::string& name) {
    Rc<IdentifierToken> token{ NewObj<IdentifierToken>() };
    token->SetName(name);
    return token;
}

static Rc<NumericLiteralToken> NewNumericLiteralToken(const std::string& wholeValue) {
    Rc<NumericLiteralToken> token{ NewObj<NumericLiteralToken>() };
    token->SetWholeValue(wholeValue);
    return token;
}

static Rc<StringLiteralToken> NewStringLiteralToken(const std::string& value) {
    Rc<StringLiteralToken> token{ NewObj<StringLiteralToken>() };
    token->SetValue(value);
    token->SetOpeningQuote('"');
    token->SetClosingQuote('"');
    return token;
}

TEST(ExpressionParserTest, EmptyFile) {
    std::vector<Rc<SyntaxToken>> tokens{ NewEofToken() };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> expression{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(!expression);
}

TEST(ExpressionParserTest, PrimaryExpression_Identifier) {
    std::string identifierName{ "FooBar" };
    std::vector<Rc<SyntaxToken>> tokens{
        NewIdentifierToken(identifierName),
        NewEofToken()
    };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> expression{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(expression);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(expression));

    Rc<PrimaryExpression> primaryExpression{ As<PrimaryExpression>(expression) };
    ASSERT_TRUE(primaryExpression->IsIdentifier());
    EXPECT_TRUE(primaryExpression->IsValid());

    Rc<SyntaxNode> value{ primaryExpression->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(value));

    Rc<IdentifierToken> identifier{ As<IdentifierToken>(value) };
    EXPECT_EQ(identifier->GetName(), identifierName);
}

TEST(ExpressionParserTest, PrimaryExpression_NumericLiteral) {
    std::string wholeValue{ "1234567890" };
    std::vector<Rc<SyntaxToken>> tokens{
        NewNumericLiteralToken(wholeValue),
        NewEofToken()
    };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> expression{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(expression);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(expression));

    Rc<PrimaryExpression> primaryExpression{ As<PrimaryExpression>(expression) };
    ASSERT_TRUE(primaryExpression->IsNumericLiteral());
    ASSERT_TRUE(primaryExpression->IsValid());

    Rc<SyntaxNode> value{ primaryExpression->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<NumericLiteralToken>(value));

    Rc<NumericLiteralToken> literalToken{ As<NumericLiteralToken>(value) };
    EXPECT_EQ(literalToken->GetWholeValue(), wholeValue);
}

TEST(ExpressionParserTest, PrimaryExpression_StringLiteral) {
    std::string literalValue{ "The quick brown fox jumps over the lazy dog." };
    std::vector<Rc<SyntaxToken>> tokens{
        NewStringLiteralToken(literalValue),
        NewEofToken()
    };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> expression{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(expression);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(expression));

    Rc<PrimaryExpression> primaryExpression{ As<PrimaryExpression>(expression) };
    ASSERT_TRUE(primaryExpression->IsStringLiteral());
    ASSERT_TRUE(primaryExpression->IsValid());

    Rc<SyntaxNode> value{ primaryExpression->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<StringLiteralToken>(value));

    Rc<StringLiteralToken> literalToken{ As<StringLiteralToken>(value) };
    EXPECT_EQ(literalToken->GetValue(), literalValue);
}

TEST(ExpressionParserTest, PostfixExpression_ArrayAccess) {
    std::string identifierName{ "FooBar" };
    std::string numericValue{ "1000" };
    std::vector<Rc<SyntaxToken>> tokens{
        NewIdentifierToken(identifierName),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(numericValue),
        NewObj<RBracketSymbol>(),
        NewObj<EofToken>()
    };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> postfixExpressionBase{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(postfixExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(postfixExpressionBase));

    Rc<PostfixExpression> postfixExpression{ As<PostfixExpression>(postfixExpressionBase) };
    ASSERT_TRUE(postfixExpression->IsArrayAccessor());
    ASSERT_TRUE(postfixExpression->IsValid());

    Rc<SyntaxNode> primaryExpressionBase{ postfixExpression->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(primaryExpressionBase));

    Rc<PrimaryExpression> primaryExpression{ As<PrimaryExpression>(primaryExpressionBase) };
    ASSERT_TRUE(primaryExpression->IsIdentifier());
    ASSERT_TRUE(primaryExpression->IsValid());

    Rc<SyntaxNode> identifierBase{ primaryExpression->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(identifierBase));

    Rc<IdentifierToken> identifier{ As<IdentifierToken>(identifierBase) };
    EXPECT_EQ(identifier->GetName(), identifierName);

    Rc<SyntaxNode> lBracketBase{ postfixExpression->GetChildren()[1] };
    ASSERT_TRUE(IsSyntaxNode<LBracketSymbol>(lBracketBase));

    Rc<SyntaxNode> indexValueBase{ postfixExpression->GetChildren()[2] };
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(indexValueBase));

    Rc<PrimaryExpression> indexValue{ As<PrimaryExpression>(indexValueBase) };
    ASSERT_TRUE(indexValue->IsNumericLiteral());
    ASSERT_TRUE(indexValue->IsValid());

    Rc<SyntaxNode> numericLiteralBase{ indexValue->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<NumericLiteralToken>(numericLiteralBase));

    Rc<NumericLiteralToken> numericLiteral{ As<NumericLiteralToken>(numericLiteralBase) };
    EXPECT_EQ(numericLiteral->GetWholeValue(), numericValue);
}

TEST(ExpressionParserTest, PostfixExpression_MemberAccess) {
    std::string objName{ "FooBar" };
    std::string memberName{ "Value" };
    std::vector<Rc<SyntaxToken>> tokens{
        NewIdentifierToken(objName),
        NewObj<DotSymbol>(),
        NewIdentifierToken(memberName),
        NewObj<EofToken>()
    };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> postfixExpressionBase{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(postfixExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(postfixExpressionBase));

    Rc<PostfixExpression> postfixExpression{ As<PostfixExpression>(postfixExpressionBase) };
    ASSERT_TRUE(postfixExpression->IsStructureReference());
    ASSERT_TRUE(postfixExpression->IsValid());

    Rc<SyntaxNode> objBase{ postfixExpression->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> dotSymbolBase{ postfixExpression->GetChildren()[1] };
    ASSERT_TRUE(IsSyntaxNode<DotSymbol>(dotSymbolBase));

    Rc<SyntaxNode> memberBase{ postfixExpression->GetChildren()[2] };
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(memberBase));

    Rc<IdentifierToken> member{ As<IdentifierToken>(memberBase) };
    EXPECT_EQ(member->GetName(), memberName);
}

TEST(ExpressionParserTest, PostfixExpression_MemberPointerAccess) {
    std::string objName{ "FooBar" };
    std::string memberName{ "Value" };
    std::vector<Rc<SyntaxToken>> tokens{
        NewIdentifierToken(objName),
        NewObj<MinusGtSymbol>(),
        NewIdentifierToken(memberName),
        NewObj<EofToken>()
    };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> postfixExpressionBase{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(postfixExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(postfixExpressionBase));

    Rc<PostfixExpression> postfixExpression{ As<PostfixExpression>(postfixExpressionBase) };
    ASSERT_TRUE(postfixExpression->IsStructureDereference());
    ASSERT_TRUE(postfixExpression->IsValid());

    Rc<SyntaxNode> objBase{ postfixExpression->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> minusGtSymbolBase{ postfixExpression->GetChildren()[1] };
    ASSERT_TRUE(IsSyntaxNode<MinusGtSymbol>(minusGtSymbolBase));

    Rc<SyntaxNode> memberBase{ postfixExpression->GetChildren()[2] };
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(memberBase));

    Rc<IdentifierToken> member{ As<IdentifierToken>(memberBase) };
    EXPECT_EQ(member->GetName(), memberName);
}

TEST(ExpressionParserTest, PostfixExpression_PostIncrement) {
    std::string objName{ "FooBar" };
    std::vector<Rc<SyntaxToken>> tokens{
        NewIdentifierToken(objName),
        NewObj<PlusPlusSymbol>(),
        NewObj<EofToken>()
    };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> postfixExpressionBase{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(postfixExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(postfixExpressionBase));

    Rc<PostfixExpression> postfixExpression{ As<PostfixExpression>(postfixExpressionBase) };
    ASSERT_TRUE(postfixExpression->IsPostIncrement());
    ASSERT_TRUE(postfixExpression->IsValid());

    Rc<SyntaxNode> objBase{ postfixExpression->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> postIncrementBase{ postfixExpression->GetChildren()[1] };
    ASSERT_TRUE(IsSyntaxNode<PlusPlusSymbol>(postIncrementBase));
}

TEST(ExpressionParserTest, PostfixExpression_PostDecrement) {
    std::string objName{ "FooBar" };
    std::vector<Rc<SyntaxToken>> tokens{
        NewIdentifierToken(objName),
        NewObj<MinusMinusSymbol>(),
        NewObj<EofToken>()
    };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> postfixExpressionBase{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(postfixExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(postfixExpressionBase));

    Rc<PostfixExpression> postfixExpression{ As<PostfixExpression>(postfixExpressionBase) };
    ASSERT_TRUE(postfixExpression->IsPostDecrement());
    ASSERT_TRUE(postfixExpression->IsValid());

    Rc<SyntaxNode> objBase{ postfixExpression->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> postDecrementBase{ postfixExpression->GetChildren()[1] };
    ASSERT_TRUE(IsSyntaxNode<MinusMinusSymbol>(postDecrementBase));
}