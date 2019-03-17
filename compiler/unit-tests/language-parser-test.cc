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
    ASSERT_TRUE(value);
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
    ASSERT_TRUE(value);
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
    ASSERT_TRUE(value);
    ASSERT_TRUE(IsSyntaxNode<StringLiteralToken>(value));

    Rc<StringLiteralToken> literalToken{ As<StringLiteralToken>(value) };
    EXPECT_EQ(literalToken->GetValue(), literalValue);
}

TEST(ExpressionParserTest, PrimaryExpression_ParenthesizedExpression) {
    std::string identifierName{ "FooBar" };
    std::vector<Rc<SyntaxToken>> tokens{
        NewObj<LParenSymbol>(),
        NewIdentifierToken(identifierName),
        NewObj<RParenSymbol>(),
        NewObj<EofToken>()
    };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> expression{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(expression);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(expression));

    Rc<PrimaryExpression> parenthesizedExpression{ As<PrimaryExpression>(expression) };
    ASSERT_TRUE(parenthesizedExpression->IsParenthesizedExpression());
    ASSERT_TRUE(parenthesizedExpression->IsValid());

    Rc<SyntaxNode> lParenBase{ parenthesizedExpression->GetChildren()[0] };
    ASSERT_TRUE(lParenBase);
    ASSERT_TRUE(IsSyntaxNode<LParenSymbol>(lParenBase));

    Rc<SyntaxNode> objBase{ parenthesizedExpression->GetChildren()[1] };
    ASSERT_TRUE(objBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(objTokenBase);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), identifierName);

    Rc<SyntaxNode> rParenBase{ parenthesizedExpression->GetChildren()[2] };
    ASSERT_TRUE(rParenBase);
    ASSERT_TRUE(IsSyntaxNode<RParenSymbol>(rParenBase));
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
    ASSERT_TRUE(primaryExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(primaryExpressionBase));

    Rc<PrimaryExpression> primaryExpression{ As<PrimaryExpression>(primaryExpressionBase) };
    ASSERT_TRUE(primaryExpression->IsIdentifier());
    ASSERT_TRUE(primaryExpression->IsValid());

    Rc<SyntaxNode> identifierBase{ primaryExpression->GetChildren()[0] };
    ASSERT_TRUE(identifierBase);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(identifierBase));

    Rc<IdentifierToken> identifier{ As<IdentifierToken>(identifierBase) };
    EXPECT_EQ(identifier->GetName(), identifierName);

    Rc<SyntaxNode> lBracketBase{ postfixExpression->GetChildren()[1] };
    ASSERT_TRUE(lBracketBase);
    ASSERT_TRUE(IsSyntaxNode<LBracketSymbol>(lBracketBase));

    Rc<SyntaxNode> indexValueBase{ postfixExpression->GetChildren()[2] };
    ASSERT_TRUE(indexValueBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(indexValueBase));

    Rc<PrimaryExpression> indexValue{ As<PrimaryExpression>(indexValueBase) };
    ASSERT_TRUE(indexValue->IsNumericLiteral());
    ASSERT_TRUE(indexValue->IsValid());

    Rc<SyntaxNode> numericLiteralBase{ indexValue->GetChildren()[0] };
    ASSERT_TRUE(numericLiteralBase);
    ASSERT_TRUE(IsSyntaxNode<NumericLiteralToken>(numericLiteralBase));

    Rc<NumericLiteralToken> numericLiteral{ As<NumericLiteralToken>(numericLiteralBase) };
    EXPECT_EQ(numericLiteral->GetWholeValue(), numericValue);
}

TEST(ExpressionParserTest, PostfixExpression_ArrayAccess_Chained) {
    std::string objName{ "FooBar" };
    std::string numericValue1{ "1000" };
    std::string numericValue2{ "2000" };
    std::vector<Rc<SyntaxToken>> tokens{
        NewIdentifierToken(objName),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(numericValue1),
        NewObj<RBracketSymbol>(),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(numericValue2),
        NewObj<RBracketSymbol>(),
        NewObj<EofToken>()
    };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> rightPostfixExpressionBase{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(rightPostfixExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(rightPostfixExpressionBase));

    Rc<PostfixExpression> rightPostfixExpression{ As<PostfixExpression>(rightPostfixExpressionBase) };
    ASSERT_TRUE(rightPostfixExpression->IsArrayAccessor());
    ASSERT_TRUE(rightPostfixExpression->IsValid());

    Rc<SyntaxNode> leftPostfixExpressionBase{ rightPostfixExpression->GetChildren()[0] };
    ASSERT_TRUE(leftPostfixExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(leftPostfixExpressionBase));

    Rc<PostfixExpression> leftPostfixExpression{ As<PostfixExpression>(leftPostfixExpressionBase) };
    ASSERT_TRUE(leftPostfixExpression->IsArrayAccessor());
    ASSERT_TRUE(leftPostfixExpression->IsValid());

    Rc<SyntaxNode> objBase{ leftPostfixExpression->GetChildren()[0] };
    ASSERT_TRUE(objBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(objTokenBase);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> leftLBracketBase{ leftPostfixExpression->GetChildren()[1] };
    ASSERT_TRUE(leftLBracketBase);
    ASSERT_TRUE(IsSyntaxNode<LBracketSymbol>(leftLBracketBase));

    Rc<SyntaxNode> leftIndexBase{ leftPostfixExpression->GetChildren()[2] };
    ASSERT_TRUE(leftIndexBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(leftIndexBase));

    Rc<PrimaryExpression> leftIndex{ As<PrimaryExpression>(leftIndexBase) };
    ASSERT_TRUE(leftIndex->IsNumericLiteral());
    ASSERT_TRUE(leftIndex->IsValid());

    Rc<SyntaxNode> leftIndexTokenBase{ leftIndex->GetChildren()[0] };
    ASSERT_TRUE(leftIndexTokenBase);
    ASSERT_TRUE(IsSyntaxNode<NumericLiteralToken>(leftIndexTokenBase));

    Rc<NumericLiteralToken> leftIndexToken{ As<NumericLiteralToken>(leftIndexTokenBase) };
    EXPECT_EQ(leftIndexToken->GetWholeValue(), numericValue1);

    Rc<SyntaxNode> leftRBracketBase{ leftPostfixExpression->GetChildren()[3] };
    ASSERT_TRUE(leftRBracketBase);
    ASSERT_TRUE(IsSyntaxNode<RBracketSymbol>(leftRBracketBase));

    Rc<SyntaxNode> rightLBracketBase{ rightPostfixExpression->GetChildren()[1] };
    ASSERT_TRUE(rightLBracketBase);
    ASSERT_TRUE(IsSyntaxNode<LBracketSymbol>(rightLBracketBase));

    Rc<SyntaxNode> rightIndexBase{ rightPostfixExpression->GetChildren()[2] };
    ASSERT_TRUE(rightIndexBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(rightIndexBase));

    Rc<PrimaryExpression> rightIndex{ As<PrimaryExpression>(rightIndexBase) };
    ASSERT_TRUE(rightIndex->IsNumericLiteral());
    ASSERT_TRUE(rightIndex->IsValid());

    Rc<SyntaxNode> rightIndexTokenBase{ rightIndex->GetChildren()[0] };
    ASSERT_TRUE(rightIndexTokenBase);
    ASSERT_TRUE(IsSyntaxNode<NumericLiteralToken>(rightIndexTokenBase));

    Rc<NumericLiteralToken> rightIndexToken{ As<NumericLiteralToken>(rightIndexTokenBase) };
    EXPECT_EQ(rightIndexToken->GetWholeValue(), numericValue2);

    Rc<SyntaxNode> rightRBracketBase{ rightPostfixExpression->GetChildren()[3] };
    ASSERT_TRUE(rightRBracketBase);
    ASSERT_TRUE(IsSyntaxNode<RBracketSymbol>(rightRBracketBase));
}

TEST(ExpressionParserTest, PostfixExpression_ArrayAccess_MissingRBracket) {
    std::string objName{ "FooBar" };
    std::string indexValue{ "1000" };
    std::vector<Rc<SyntaxToken>> tokens{
        NewIdentifierToken(objName),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(indexValue),
        NewObj<EofToken>(),
    };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> postfixExpressionBase{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(postfixExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(postfixExpressionBase));

    Rc<PostfixExpression> postfixExpression{ As<PostfixExpression>(postfixExpressionBase) };
    ASSERT_TRUE(postfixExpression->IsArrayAccessor());
    ASSERT_TRUE(!postfixExpression->IsValid());

    Rc<SyntaxNode> objBase{ postfixExpression->GetChildren()[0] };
    ASSERT_TRUE(objBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(objTokenBase);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> lBracketBase{ postfixExpression->GetChildren()[1] };
    ASSERT_TRUE(lBracketBase);
    ASSERT_TRUE(IsSyntaxNode<LBracketSymbol>(lBracketBase));

    Rc<SyntaxNode> indexBase{ postfixExpression->GetChildren()[2] };
    ASSERT_TRUE(indexBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(indexBase));

    Rc<PrimaryExpression> index{ As<PrimaryExpression>(indexBase) };
    ASSERT_TRUE(index->IsIdentifier());
    ASSERT_TRUE(index->IsValid());

    Rc<SyntaxNode> indexTokenBase{ index->GetChildren()[0] };
    ASSERT_TRUE(indexTokenBase);
    ASSERT_TRUE(IsSyntaxNode<NumericLiteralToken>(indexTokenBase));

    Rc<NumericLiteralToken> indexToken{ As<NumericLiteralToken>(indexTokenBase) };
    EXPECT_EQ(indexToken->GetWholeValue(), indexValue);

    Rc<SyntaxNode> rBracketBase{ postfixExpression->GetChildren()[3] };
    ASSERT_TRUE(rBracketBase);
    ASSERT_TRUE(IsSyntaxNode<RBracketSymbol>(rBracketBase));

    Rc<RBracketSymbol> rBracket{ As<RBracketSymbol>(rBracketBase) };
    EXPECT_TRUE(rBracket->GetFlags() & SyntaxToken::IS_MISSING);
}

TEST(ExpressionParserTest, PostfixExpression_ArrayAccess_MissingFirstRBracket_Chained) {
    std::string objName{ "FooBar" };
    std::string indexValue1{ "1000" };
    std::string indexValue2{ "2000" };
    std::vector<Rc<SyntaxToken>> tokens{
        NewIdentifierToken(objName),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(indexValue1),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(indexValue2),
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
    ASSERT_TRUE(!postfixExpression->IsValid());

    Rc<SyntaxNode> objBase{ postfixExpression->GetChildren()[0] };
    ASSERT_TRUE(objBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(objTokenBase);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> lBracketBase{ postfixExpression->GetChildren()[1] };
    ASSERT_TRUE(lBracketBase);
    ASSERT_TRUE(IsSyntaxNode<LBracketSymbol>(lBracketBase));

    Rc<SyntaxNode> indexBase{ postfixExpression->GetChildren()[2] };
    ASSERT_TRUE(indexBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(indexBase));

    Rc<PrimaryExpression> index{ As<PrimaryExpression>(indexBase) };
    ASSERT_TRUE(index->IsIdentifier());
    ASSERT_TRUE(index->IsValid());

    Rc<SyntaxNode> indexTokenBase{ index->GetChildren()[0] };
    ASSERT_TRUE(indexTokenBase);
    ASSERT_TRUE(IsSyntaxNode<NumericLiteralToken>(indexTokenBase));

    Rc<NumericLiteralToken> indexToken{ As<NumericLiteralToken>(indexTokenBase) };
    EXPECT_EQ(indexToken->GetWholeValue(), indexValue1);

    Rc<SyntaxNode> rBracketBase{ postfixExpression->GetChildren()[3] };
    ASSERT_TRUE(rBracketBase);
    ASSERT_TRUE(IsSyntaxNode<RBracketSymbol>(rBracketBase));

    Rc<RBracketSymbol> rBracket{ As<RBracketSymbol>(rBracketBase) };
    EXPECT_TRUE(rBracket->GetFlags() & SyntaxToken::IS_MISSING);
}

TEST(ExpressionParserTest, PostfixExpression_ArrayAccess_MissingSecondRBracket_Chained) {
    std::string objName{ "FooBar" };
    std::string indexValue1{ "1000" };
    std::string indexValue2{ "2000" };
    std::vector<Rc<SyntaxToken>> tokens{
        NewIdentifierToken(objName),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(indexValue1),
        NewObj<RBracketSymbol>(),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(indexValue2),
        NewObj<EofToken>()
    };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> rightPostfixExpressionBase{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(rightPostfixExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(rightPostfixExpressionBase));

    Rc<PostfixExpression> rightPostfixExpression{ As<PostfixExpression>(rightPostfixExpressionBase) };
    ASSERT_TRUE(rightPostfixExpression->IsArrayAccessor());
    ASSERT_TRUE(!rightPostfixExpression->IsValid());

    Rc<SyntaxNode> leftPostfixExpressionBase{ rightPostfixExpression->GetChildren()[0] };
    ASSERT_TRUE(leftPostfixExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(leftPostfixExpressionBase));

    Rc<PostfixExpression> leftPostfixExpression{ As<PostfixExpression>(leftPostfixExpressionBase) };
    ASSERT_TRUE(leftPostfixExpression->IsArrayAccessor());
    ASSERT_TRUE(leftPostfixExpression->IsValid());

    Rc<SyntaxNode> objBase{ leftPostfixExpression->GetChildren()[0] };
    ASSERT_TRUE(objBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(objTokenBase);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> leftLBracketBase{ leftPostfixExpression->GetChildren()[1] };
    ASSERT_TRUE(leftLBracketBase);
    ASSERT_TRUE(IsSyntaxNode<LBracketSymbol>(leftLBracketBase));

    Rc<SyntaxNode> leftIndexBase{ leftPostfixExpression->GetChildren()[2] };
    ASSERT_TRUE(leftIndexBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(leftIndexBase));

    Rc<PrimaryExpression> leftIndex{ As<PrimaryExpression>(leftIndexBase) };
    ASSERT_TRUE(leftIndex->IsNumericLiteral());
    ASSERT_TRUE(leftIndex->IsValid());

    Rc<SyntaxNode> leftIndexTokenBase{ leftIndex->GetChildren()[0] };
    ASSERT_TRUE(leftIndexTokenBase);
    ASSERT_TRUE(IsSyntaxNode<NumericLiteralToken>(leftIndexTokenBase));

    Rc<NumericLiteralToken> leftIndexToken{ As<NumericLiteralToken>(leftIndexTokenBase) };
    EXPECT_EQ(leftIndexToken->GetWholeValue(), indexValue1);

    Rc<SyntaxNode> leftRBracketBase{ leftPostfixExpression->GetChildren()[3] };
    ASSERT_TRUE(leftRBracketBase);
    ASSERT_TRUE(IsSyntaxNode<RBracketSymbol>(leftRBracketBase));

    Rc<SyntaxNode> rightLBracketBase{ rightPostfixExpression->GetChildren()[1] };
    ASSERT_TRUE(rightLBracketBase);
    ASSERT_TRUE(IsSyntaxNode<LBracketSymbol>(rightLBracketBase));

    Rc<SyntaxNode> rightIndexBase{ rightPostfixExpression->GetChildren()[2] };
    ASSERT_TRUE(rightIndexBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(rightIndexBase));

    Rc<PrimaryExpression> rightIndex{ As<PrimaryExpression>(rightIndexBase) };
    ASSERT_TRUE(rightIndex->IsNumericLiteral());
    ASSERT_TRUE(rightIndex->IsValid());

    Rc<SyntaxNode> rightIndexTokenBase{ rightIndex->GetChildren()[0] };
    ASSERT_TRUE(rightIndexTokenBase);
    ASSERT_TRUE(IsSyntaxNode<NumericLiteralToken>(rightIndexTokenBase));

    Rc<NumericLiteralToken> rightIndexToken{ As<NumericLiteralToken>(rightIndexTokenBase) };
    EXPECT_EQ(rightIndexToken->GetWholeValue(), indexValue2);

    Rc<SyntaxNode> rightRBracketBase{ rightPostfixExpression->GetChildren()[3] };
    ASSERT_TRUE(rightRBracketBase);
    ASSERT_TRUE(IsSyntaxNode<RBracketSymbol>(rightRBracketBase));

    Rc<RBracketSymbol> rightRBracket{ As<RBracketSymbol>(rightRBracketBase) };
    EXPECT_TRUE(rightRBracket->GetFlags() & SyntaxToken::IS_MISSING);
}

TEST(ExpressionParserTest, PostfixExpression_ArrayAccess_MissingBothRBrackets_Chained) {
    std::string objName{ "FooBar" };
    std::string indexValue1{ "1000" };
    std::string indexValue2{ "2000" };
    std::vector<Rc<SyntaxToken>> tokens{
        NewIdentifierToken(objName),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(indexValue1),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(indexValue2),
        NewObj<EofToken>()
    };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> postfixExpressionBase{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(postfixExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(postfixExpressionBase));

    Rc<PostfixExpression> postfixExpression{ As<PostfixExpression>(postfixExpressionBase) };
    ASSERT_TRUE(postfixExpression->IsArrayAccessor());
    ASSERT_TRUE(!postfixExpression->IsValid());

    Rc<SyntaxNode> objBase{ postfixExpression->GetChildren()[0] };
    ASSERT_TRUE(objBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(objTokenBase);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> lBracketBase{ postfixExpression->GetChildren()[1] };
    ASSERT_TRUE(lBracketBase);
    ASSERT_TRUE(IsSyntaxNode<LBracketSymbol>(lBracketBase));

    Rc<SyntaxNode> indexBase{ postfixExpression->GetChildren()[2] };
    ASSERT_TRUE(indexBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(indexBase));

    Rc<PrimaryExpression> index{ As<PrimaryExpression>(indexBase) };
    ASSERT_TRUE(index->IsIdentifier());
    ASSERT_TRUE(index->IsValid());

    Rc<SyntaxNode> indexTokenBase{ index->GetChildren()[0] };
    ASSERT_TRUE(indexTokenBase);
    ASSERT_TRUE(IsSyntaxNode<NumericLiteralToken>(indexTokenBase));

    Rc<NumericLiteralToken> indexToken{ As<NumericLiteralToken>(indexTokenBase) };
    EXPECT_EQ(indexToken->GetWholeValue(), indexValue1);

    Rc<SyntaxNode> rBracketBase{ postfixExpression->GetChildren()[3] };
    ASSERT_TRUE(rBracketBase);
    ASSERT_TRUE(IsSyntaxNode<RBracketSymbol>(rBracketBase));

    Rc<RBracketSymbol> rBracket{ As<RBracketSymbol>(rBracket) };
    EXPECT_TRUE(rBracket->GetFlags() & SyntaxToken::IS_MISSING);
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
    ASSERT_TRUE(objBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(objTokenBase);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> dotSymbolBase{ postfixExpression->GetChildren()[1] };
    ASSERT_TRUE(dotSymbolBase);
    ASSERT_TRUE(IsSyntaxNode<DotSymbol>(dotSymbolBase));

    Rc<SyntaxNode> memberBase{ postfixExpression->GetChildren()[2] };
    ASSERT_TRUE(memberBase);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(memberBase));

    Rc<IdentifierToken> member{ As<IdentifierToken>(memberBase) };
    EXPECT_EQ(member->GetName(), memberName);
}

TEST(ExpressionParserTest, PostfixExpression_MemberAccess_Chained) {
    std::string objName{ "FooBar" };
    std::string memberName1{ "Member1" };
    std::string memberName2{ "Member2" };
    std::vector<Rc<SyntaxToken>> tokens{
        NewIdentifierToken(objName),
        NewObj<DotSymbol>(),
        NewIdentifierToken(memberName1),
        NewObj<DotSymbol>(),
        NewIdentifierToken(memberName2),
        NewObj<EofToken>()
    };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> rightPostfixExpressionBase{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(rightPostfixExpressionBase));

    Rc<PostfixExpression> rightPostfixExpression{ As<PostfixExpression>(rightPostfixExpressionBase) };
    ASSERT_TRUE(rightPostfixExpression->IsStructureReference());
    ASSERT_TRUE(rightPostfixExpression->IsValid());

    Rc<SyntaxNode> leftPostfixExpressionBase{ rightPostfixExpression->GetChildren()[0] };
    ASSERT_TRUE(leftPostfixExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(leftPostfixExpressionBase));

    Rc<PostfixExpression> leftPostfixExpression{ As<PostfixExpression>(leftPostfixExpressionBase) };
    ASSERT_TRUE(leftPostfixExpression->IsStructureReference());
    ASSERT_TRUE(leftPostfixExpression->IsValid());

    Rc<SyntaxNode> objBase{ leftPostfixExpression->GetChildren()[0] };
    ASSERT_TRUE(objBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(objTokenBase);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> leftDotSymbolBase{ leftPostfixExpression->GetChildren()[1] };
    ASSERT_TRUE(leftDotSymbolBase);
    ASSERT_TRUE(IsSyntaxNode<DotSymbol>(leftDotSymbolBase));

    Rc<SyntaxNode> memberBase1{ leftPostfixExpression->GetChildren()[2] };
    ASSERT_TRUE(memberBase1);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(memberBase1));

    Rc<IdentifierToken> member1{ As<IdentifierToken>(memberBase1) };
    EXPECT_EQ(member1->GetName(), memberName1);

    Rc<SyntaxNode> rightDotSymbolBase{ rightPostfixExpression->GetChildren()[1] };
    ASSERT_TRUE(rightDotSymbolBase);
    ASSERT_TRUE(IsSyntaxNode<DotSymbol>(rightDotSymbolBase));

    Rc<SyntaxNode> memberBase2{ rightPostfixExpression->GetChildren()[2] };
    ASSERT_TRUE(memberBase2);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(memberBase2));

    Rc<IdentifierToken> member2{ As<IdentifierToken>(memberBase2) };
    EXPECT_EQ(member2->GetName(), memberName2);
}

TEST(ExpressionParserTest, PostfixExpression_MemberAccess_MissingMemberName) {
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
    ASSERT_TRUE(!postfixExpression->IsValid());

    Rc<SyntaxNode> objBase{ postfixExpression->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> accessorBase{ postfixExpression->GetChildren()[1] };
    ASSERT_TRUE(accessorBase);
    ASSERT_TRUE(IsSyntaxNode<DotSymbol>(accessorBase));

    Rc<SyntaxNode> memberBase{ postfixExpression->GetChildren()[2] };
    ASSERT_TRUE(!memberBase);
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
    ASSERT_TRUE(objBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(objTokenBase);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> minusGtSymbolBase{ postfixExpression->GetChildren()[1] };
    ASSERT_TRUE(minusGtSymbolBase);
    ASSERT_TRUE(IsSyntaxNode<MinusGtSymbol>(minusGtSymbolBase));

    Rc<SyntaxNode> memberBase{ postfixExpression->GetChildren()[2] };
    ASSERT_TRUE(memberBase);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(memberBase));

    Rc<IdentifierToken> member{ As<IdentifierToken>(memberBase) };
    EXPECT_EQ(member->GetName(), memberName);
}

TEST(ExpressionParserTest, PostfixExpression_MemberPointerAccess_Chained) {
    std::string objName{ "FooBar" };
    std::string memberName1{ "Member1" };
    std::string memberName2{ "Member2" };
    std::vector<Rc<SyntaxToken>> tokens{
        NewIdentifierToken(objName),
        NewObj<MinusGtSymbol>(),
        NewIdentifierToken(memberName1),
        NewObj<MinusGtSymbol>(),
        NewIdentifierToken(memberName2),
        NewObj<EofToken>()
    };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> rightPostfixExpressionBase{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(rightPostfixExpressionBase));

    Rc<PostfixExpression> rightPostfixExpression{ As<PostfixExpression>(rightPostfixExpressionBase) };
    ASSERT_TRUE(rightPostfixExpression->IsStructureDereference());
    ASSERT_TRUE(rightPostfixExpression->IsValid());

    Rc<SyntaxNode> leftPostfixExpressionBase{ rightPostfixExpression->GetChildren()[0] };
    ASSERT_TRUE(leftPostfixExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(leftPostfixExpressionBase));

    Rc<PostfixExpression> leftPostfixExpression{ As<PostfixExpression>(leftPostfixExpressionBase) };
    ASSERT_TRUE(leftPostfixExpression->IsStructureDereference());
    ASSERT_TRUE(leftPostfixExpression->IsValid());

    Rc<SyntaxNode> objBase{ leftPostfixExpression->GetChildren()[0] };
    ASSERT_TRUE(objBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(objTokenBase);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> leftArrowSymbolBase{ leftPostfixExpression->GetChildren()[1] };
    ASSERT_TRUE(leftArrowSymbolBase);
    ASSERT_TRUE(IsSyntaxNode<MinusGtSymbol>(leftArrowSymbolBase));

    Rc<SyntaxNode> memberBase1{ leftPostfixExpression->GetChildren()[2] };
    ASSERT_TRUE(memberBase1);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(memberBase1));

    Rc<IdentifierToken> member1{ As<IdentifierToken>(memberBase1) };
    EXPECT_EQ(member1->GetName(), memberName1);

    Rc<SyntaxNode> rightArrowSymbolBase{ rightPostfixExpression->GetChildren()[1] };
    ASSERT_TRUE(rightArrowSymbolBase);
    ASSERT_TRUE(IsSyntaxNode<MinusGtSymbol>(rightArrowSymbolBase));

    Rc<SyntaxNode> memberBase2{ rightPostfixExpression->GetChildren()[2] };
    ASSERT_TRUE(memberBase2);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(memberBase2));

    Rc<IdentifierToken> member2{ As<IdentifierToken>(memberBase2) };
    EXPECT_EQ(member2->GetName(), memberName2);
}

TEST(ExpressionParserTest, PostfixExpression_MemberPointerAccess_MissingMemberName) {
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
    ASSERT_TRUE(!postfixExpression->IsValid());

    Rc<SyntaxNode> objBase{ postfixExpression->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> accessorBase{ postfixExpression->GetChildren()[1] };
    ASSERT_TRUE(accessorBase);
    ASSERT_TRUE(IsSyntaxNode<MinusGtSymbol>(accessorBase));

    Rc<SyntaxNode> memberBase{ postfixExpression->GetChildren()[2] };
    ASSERT_TRUE(!memberBase);
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
    ASSERT_TRUE(objBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(objTokenBase);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> postIncrementBase{ postfixExpression->GetChildren()[1] };
    ASSERT_TRUE(postIncrementBase);
    ASSERT_TRUE(IsSyntaxNode<PlusPlusSymbol>(postIncrementBase));
}

TEST(ExpressionParserTest, PostfixExpression_PostIncrement_Chained) {
    std::string objName{ "FooBar" };
    std::vector<Rc<SyntaxToken>> tokens{
        NewIdentifierToken(objName),
        NewObj<PlusPlusSymbol>(),
        NewObj<PlusPlusSymbol>(),
        NewObj<EofToken>()
    };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> rightPostfixExpressionBase{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(rightPostfixExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(rightPostfixExpressionBase));

    Rc<PostfixExpression> rightPostfixExpression{ As<PostfixExpression>(rightPostfixExpressionBase) };
    ASSERT_TRUE(rightPostfixExpression->IsPostIncrement());
    ASSERT_TRUE(rightPostfixExpression->IsValid());

    Rc<SyntaxNode> leftPostfixExpressionBase{ rightPostfixExpression->GetChildren()[0] };
    ASSERT_TRUE(leftPostfixExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(leftPostfixExpressionBase));

    Rc<PostfixExpression> leftPostfixExpression{ As<PostfixExpression>(leftPostfixExpressionBase) };
    ASSERT_TRUE(leftPostfixExpression->IsPostIncrement());
    ASSERT_TRUE(leftPostfixExpression->IsValid());

    Rc<SyntaxNode> objBase{ leftPostfixExpression->GetChildren()[0] };
    ASSERT_TRUE(objBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(objTokenBase);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> firstIncrement{ leftPostfixExpression->GetChildren()[1] };
    ASSERT_TRUE(firstIncrement);
    ASSERT_TRUE(IsSyntaxNode<PlusPlusSymbol>(firstIncrement));

    Rc<SyntaxNode> secondIncrement{ rightPostfixExpression->GetChildren()[1] };
    ASSERT_TRUE(secondIncrement);
    ASSERT_TRUE(IsSyntaxNode<PlusPlusSymbol>(secondIncrement));
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
    ASSERT_TRUE(objBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(objTokenBase);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> postDecrementBase{ postfixExpression->GetChildren()[1] };
    ASSERT_TRUE(postDecrementBase);
    ASSERT_TRUE(IsSyntaxNode<MinusMinusSymbol>(postDecrementBase));
}

TEST(ExpressionParserTest, PostfixExpression_PostDecrement_Chained) {
    std::string objName{ "FooBar" };
    std::vector<Rc<SyntaxToken>> tokens{
        NewIdentifierToken(objName),
        NewObj<MinusMinusSymbol>(),
        NewObj<MinusMinusSymbol>(),
        NewObj<EofToken>()
    };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> rightPostfixExpressionBase{ ParseExpression(backtrackingLexer) };
    ASSERT_TRUE(rightPostfixExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(rightPostfixExpressionBase));

    Rc<PostfixExpression> rightPostfixExpression{ As<PostfixExpression>(rightPostfixExpressionBase) };
    ASSERT_TRUE(rightPostfixExpression->IsPostDecrement());
    ASSERT_TRUE(rightPostfixExpression->IsValid());

    Rc<SyntaxNode> leftPostfixExpressionBase{ rightPostfixExpression->GetChildren()[0] };
    ASSERT_TRUE(leftPostfixExpressionBase);
    ASSERT_TRUE(IsSyntaxNode<PostfixExpression>(leftPostfixExpressionBase));

    Rc<PostfixExpression> leftPostfixExpression{ As<PostfixExpression>(leftPostfixExpressionBase) };
    ASSERT_TRUE(leftPostfixExpression->IsPostDecrement());
    ASSERT_TRUE(leftPostfixExpression->IsValid());

    Rc<SyntaxNode> objBase{ leftPostfixExpression->GetChildren()[0] };
    ASSERT_TRUE(objBase);
    ASSERT_TRUE(IsSyntaxNode<PrimaryExpression>(objBase));

    Rc<PrimaryExpression> obj{ As<PrimaryExpression>(objBase) };
    ASSERT_TRUE(obj->IsIdentifier());
    ASSERT_TRUE(obj->IsValid());

    Rc<SyntaxNode> objTokenBase{ obj->GetChildren()[0] };
    ASSERT_TRUE(objTokenBase);
    ASSERT_TRUE(IsSyntaxNode<IdentifierToken>(objTokenBase));

    Rc<IdentifierToken> objToken{ As<IdentifierToken>(objTokenBase) };
    EXPECT_EQ(objToken->GetName(), objName);

    Rc<SyntaxNode> firstDecrement{ leftPostfixExpression->GetChildren()[1] };
    ASSERT_TRUE(firstDecrement);
    ASSERT_TRUE(IsSyntaxNode<MinusMinusSymbol>(firstDecrement));

    Rc<SyntaxNode> secondDecrement{ rightPostfixExpression->GetChildren()[1] };
    ASSERT_TRUE(secondDecrement);
    ASSERT_TRUE(IsSyntaxNode<MinusMinusSymbol>(secondDecrement));
}