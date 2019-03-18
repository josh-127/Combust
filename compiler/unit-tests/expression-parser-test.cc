#include <catch.hpp>
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

template<typename T>
static inline Rc<T> M(Rc<SyntaxNode> node) {
    REQUIRE(node);
    REQUIRE(IsSyntaxNode<T>(node));
    return As<T>(node);
}

template<typename T>
static Rc<T> Setup(const std::vector<Rc<SyntaxToken>>& tokens) {
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> expression{ ParseExpression(backtrackingLexer) };
    return M<T>(expression);
}

TEST_CASE("ExpressionParser PrimaryExpression EmptyFile") {
    std::vector<Rc<SyntaxToken>> tokens{ NewObj<EofToken>() };
    Rc<MockLexer> mockLexer{ NewObj<MockLexer>(tokens) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(mockLexer) };

    Rc<Expression> expression{ ParseExpression(backtrackingLexer) };
    REQUIRE(!expression);
}

TEST_CASE("ExpressionParser PrimaryExpression Identifier") {
    std::string identifierName{ "FooBar" };
    Rc<PrimaryExpression> expression{ Setup<PrimaryExpression>({
        NewIdentifierToken(identifierName),
        NewObj<EofToken>()
    }) };
    REQUIRE(expression->IsIdentifier());
    REQUIRE(expression->IsValid());

    Rc<IdentifierToken> identifier{ M<IdentifierToken>(expression->GetChildren()[0]) };
    REQUIRE(identifier->GetName() == identifierName);
}

TEST_CASE("ExpressionParser PrimaryExpression NumericLiteral") {
    std::string wholeValue{ "1234567890" };
    Rc<PrimaryExpression> expression{ Setup<PrimaryExpression>({
        NewNumericLiteralToken(wholeValue),
        NewObj<EofToken>()
    }) };
    REQUIRE(expression->IsNumericLiteral());
    REQUIRE(expression->IsValid());

    Rc<NumericLiteralToken> identifier{ M<NumericLiteralToken>(expression->GetChildren()[0]) };
    REQUIRE(identifier->GetWholeValue() == wholeValue);
}

TEST_CASE("ExpressionParser PrimaryExpression StringLiteral") {
    std::string literalValue{ "The quick brown fox jumps over the lazy dog." };
    Rc<PrimaryExpression> expression{ Setup<PrimaryExpression>({
        NewStringLiteralToken(literalValue),
        NewObj<EofToken>()
    }) };
    REQUIRE(expression->IsStringLiteral());
    REQUIRE(expression->IsValid());

    Rc<StringLiteralToken> literalToken{ M<StringLiteralToken>(expression->GetChildren()[0]) };
    REQUIRE(literalToken->GetValue() == literalValue);
}

TEST_CASE("ExpressionParser PrimaryExpression ParenthesizedExpression") {
    std::string identifierName{ "FooBar" };
    Rc<PrimaryExpression> expression{ Setup<PrimaryExpression>({
        NewObj<LParenSymbol>(),
        NewIdentifierToken(identifierName),
        NewObj<RParenSymbol>(),
        NewObj<EofToken>()
    }) };
    REQUIRE(expression->IsParenthesizedExpression());
    REQUIRE(expression->IsValid());

    M<LParenSymbol>(expression->GetChildren()[0]);

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(expression->GetChildren()[1]) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChildren()[0]) };
    REQUIRE(objToken->GetName() == identifierName);

    M<RParenSymbol>(expression->GetChildren()[2]);
}

TEST_CASE("ExpressionParser PostfixExpression ArrayAccess") {
    std::string identifierName{ "FooBar" };
    std::string numericValue{ "1000" };
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>({
        NewIdentifierToken(identifierName),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(numericValue),
        NewObj<RBracketSymbol>(),
        NewObj<EofToken>()
    }) };
    REQUIRE(postfixExpression->IsArrayAccessor());
    REQUIRE(postfixExpression->IsValid());

    Rc<PrimaryExpression> primaryExpression{ M<PrimaryExpression>(postfixExpression->GetChildren()[0]) };
    REQUIRE(primaryExpression->IsIdentifier());
    REQUIRE(primaryExpression->IsValid());

    Rc<IdentifierToken> identifier{ M<IdentifierToken>(primaryExpression->GetChildren()[0]) };
    REQUIRE(identifier->GetName() == identifierName);

    M<LBracketSymbol>(postfixExpression->GetChildren()[1]);

    Rc<PrimaryExpression> indexValue{ M<PrimaryExpression>(postfixExpression->GetChildren()[2]) };
    REQUIRE(indexValue->IsNumericLiteral());
    REQUIRE(indexValue->IsValid());

    Rc<NumericLiteralToken> numericLiteral{ M<NumericLiteralToken>(indexValue->GetChildren()[0]) };
    REQUIRE(numericLiteral->GetWholeValue() == numericValue);
}

TEST_CASE("ExpressionParser PostfixExpression ArrayAccess_Chained") {
    std::string objName{ "FooBar" };
    std::string numericValue1{ "1000" };
    std::string numericValue2{ "2000" };
    Rc<PostfixExpression> rightPostfixExpression{ Setup<PostfixExpression>({
        NewIdentifierToken(objName),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(numericValue1),
        NewObj<RBracketSymbol>(),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(numericValue2),
        NewObj<RBracketSymbol>(),
        NewObj<EofToken>()
    }) };
    REQUIRE(rightPostfixExpression->IsArrayAccessor());
    REQUIRE(rightPostfixExpression->IsValid());

    Rc<PostfixExpression> leftPostfixExpression{
        M<PostfixExpression>(rightPostfixExpression->GetChildren()[0])
    };
    REQUIRE(leftPostfixExpression->IsArrayAccessor());
    REQUIRE(leftPostfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(leftPostfixExpression->GetChildren()[0]) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChildren()[0]) };
    REQUIRE(objToken->GetName() == objName);

    M<LBracketSymbol>(leftPostfixExpression->GetChildren()[1]);

    Rc<PrimaryExpression> leftIndex{ M<PrimaryExpression>(leftPostfixExpression->GetChildren()[2]) };
    REQUIRE(leftIndex->IsNumericLiteral());
    REQUIRE(leftIndex->IsValid());

    Rc<NumericLiteralToken> leftIndexToken{ M<NumericLiteralToken>(leftIndex->GetChildren()[0]) };
    REQUIRE(leftIndexToken->GetWholeValue() == numericValue1);

    M<RBracketSymbol>(leftPostfixExpression->GetChildren()[3]);

    M<LBracketSymbol>(rightPostfixExpression->GetChildren()[1]);

    Rc<PrimaryExpression> rightIndex{ M<PrimaryExpression>(rightPostfixExpression->GetChildren()[2]) };
    REQUIRE(rightIndex->IsNumericLiteral());
    REQUIRE(rightIndex->IsValid());

    Rc<NumericLiteralToken> rightIndexToken{ M<NumericLiteralToken>(rightIndex->GetChildren()[0]) };
    REQUIRE(rightIndexToken->GetWholeValue() == numericValue2);

    M<RBracketSymbol>(rightPostfixExpression->GetChildren()[3]);
}

TEST_CASE("ExpressionParser PostfixExpression ArrayAccess_MissingRBracket") {
    std::string objName{ "FooBar" };
    std::string indexValue{ "1000" };
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>({
        NewIdentifierToken(objName),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(indexValue),
        NewObj<EofToken>(),
    }) };
    REQUIRE(postfixExpression->IsArrayAccessor());
    REQUIRE(!postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChildren()[0]) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChildren()[0]) };
    REQUIRE(objToken->GetName() == objName);

    M<LBracketSymbol>(postfixExpression->GetChildren()[1]);

    Rc<PrimaryExpression> index{ M<PrimaryExpression>(postfixExpression->GetChildren()[2]) };
    REQUIRE(index->IsIdentifier());
    REQUIRE(index->IsValid());

    Rc<NumericLiteralToken> indexToken{ M<NumericLiteralToken>(index->GetChildren()[0]) };
    REQUIRE(indexToken->GetWholeValue() == indexValue);

    Rc<RBracketSymbol> rBracket{ M<RBracketSymbol>(postfixExpression->GetChildren()[3]) };
    REQUIRE((rBracket->GetFlags() & SyntaxToken::IS_MISSING) != 0);
}

TEST_CASE("ExpressionParser PostfixExpression ArrayAccess_MissingFirstRBracket_Chained") {
    std::string objName{ "FooBar" };
    std::string indexValue1{ "1000" };
    std::string indexValue2{ "2000" };
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>({
        NewIdentifierToken(objName),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(indexValue1),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(indexValue2),
        NewObj<RBracketSymbol>(),
        NewObj<EofToken>()
    }) };
    REQUIRE(postfixExpression->IsArrayAccessor());
    REQUIRE(!postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChildren()[0]) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChildren()[0]) };
    REQUIRE(objToken->GetName() == objName);

    M<LBracketSymbol>(postfixExpression->GetChildren()[1]);

    Rc<PrimaryExpression> index{ M<PrimaryExpression>(postfixExpression->GetChildren()[2]) };
    REQUIRE(index->IsIdentifier());
    REQUIRE(index->IsValid());

    Rc<NumericLiteralToken> indexToken{ M<NumericLiteralToken>(index->GetChildren()[0]) };
    REQUIRE(indexToken->GetWholeValue() == indexValue1);

    Rc<RBracketSymbol> rBracket{ M<RBracketSymbol>(postfixExpression->GetChildren()[3]) };
    REQUIRE((rBracket->GetFlags() & SyntaxToken::IS_MISSING) != 0);
}

TEST_CASE("ExpressionParser PostfixExpression ArrayAccess_MissingSecondRBracket_Chained") {
    std::string objName{ "FooBar" };
    std::string indexValue1{ "1000" };
    std::string indexValue2{ "2000" };
    Rc<PostfixExpression> rightPostfixExpression{ Setup<PostfixExpression>({
        NewIdentifierToken(objName),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(indexValue1),
        NewObj<RBracketSymbol>(),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(indexValue2),
        NewObj<EofToken>()
    }) };
    REQUIRE(rightPostfixExpression->IsArrayAccessor());
    REQUIRE(!rightPostfixExpression->IsValid());

    Rc<PostfixExpression> leftPostfixExpression{
        M<PostfixExpression>(rightPostfixExpression->GetChildren()[0])
    };
    REQUIRE(leftPostfixExpression->IsArrayAccessor());
    REQUIRE(leftPostfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(leftPostfixExpression->GetChildren()[0]) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChildren()[0]) };
    REQUIRE(objToken->GetName() == objName);

    M<LBracketSymbol>(leftPostfixExpression->GetChildren()[1]);

    Rc<PrimaryExpression> leftIndex{ M<PrimaryExpression>(leftPostfixExpression->GetChildren()[2]) };
    REQUIRE(leftIndex->IsNumericLiteral());
    REQUIRE(leftIndex->IsValid());

    Rc<NumericLiteralToken> leftIndexToken{ M<NumericLiteralToken>(leftIndex->GetChildren()[0]) };
    REQUIRE(leftIndexToken->GetWholeValue() == indexValue1);

    M<RBracketSymbol>(leftPostfixExpression->GetChildren()[3]);

    M<LBracketSymbol>(rightPostfixExpression->GetChildren()[1]);

    Rc<PrimaryExpression> rightIndex{ M<PrimaryExpression>(rightPostfixExpression->GetChildren()[2]) };
    REQUIRE(rightIndex->IsNumericLiteral());
    REQUIRE(rightIndex->IsValid());

    Rc<NumericLiteralToken> rightIndexToken{ M<NumericLiteralToken>(rightIndex->GetChildren()[0]) };
    REQUIRE(rightIndexToken->GetWholeValue() == indexValue2);

    Rc<RBracketSymbol> rightRBracket{
        M<RBracketSymbol>(rightPostfixExpression->GetChildren()[3])
    };
    REQUIRE((rightRBracket->GetFlags() & SyntaxToken::IS_MISSING) != 0);
}

TEST_CASE("ExpressionParser PostfixExpression ArrayAccess_MissingBothRBrackets_Chained") {
    std::string objName{ "FooBar" };
    std::string indexValue1{ "1000" };
    std::string indexValue2{ "2000" };
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>({
        NewIdentifierToken(objName),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(indexValue1),
        NewObj<LBracketSymbol>(),
        NewNumericLiteralToken(indexValue2),
        NewObj<EofToken>()
    }) };
    REQUIRE(postfixExpression->IsArrayAccessor());
    REQUIRE(!postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChildren()[0]) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChildren()[0]) };
    REQUIRE(objToken->GetName() == objName);

    M<LBracketSymbol>(postfixExpression->GetChildren()[1]);

    Rc<PrimaryExpression> index{ M<PrimaryExpression>(postfixExpression->GetChildren()[2]) };
    REQUIRE(index->IsIdentifier());
    REQUIRE(index->IsValid());

    Rc<NumericLiteralToken> indexToken{ M<NumericLiteralToken>(index->GetChildren()[0]) };
    REQUIRE(indexToken->GetWholeValue() == indexValue1);

    Rc<RBracketSymbol> rBracket{ M<RBracketSymbol>(postfixExpression->GetChildren()[3]) };
    REQUIRE((rBracket->GetFlags() & SyntaxToken::IS_MISSING) != 0);
}

TEST_CASE("ExpressionParser PostfixExpression MemberAccess") {
    std::string objName{ "FooBar" };
    std::string memberName{ "Value" };
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>({
        NewIdentifierToken(objName),
        NewObj<DotSymbol>(),
        NewIdentifierToken(memberName),
        NewObj<EofToken>()
    }) };
    REQUIRE(postfixExpression->IsStructureReference());
    REQUIRE(postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChildren()[0]) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChildren()[0]) };
    REQUIRE(objToken->GetName() == objName);

    M<DotSymbol>(postfixExpression->GetChildren()[1]);

    Rc<IdentifierToken> member{ M<IdentifierToken>(postfixExpression->GetChildren()[2]) };
    REQUIRE(member->GetName() == memberName);
}

TEST_CASE("ExpressionParser PostfixExpression MemberAccess_Chained") {
    std::string objName{ "FooBar" };
    std::string memberName1{ "Member1" };
    std::string memberName2{ "Member2" };
    Rc<PostfixExpression> rightPostfixExpression{ Setup<PostfixExpression>({
        NewIdentifierToken(objName),
        NewObj<DotSymbol>(),
        NewIdentifierToken(memberName1),
        NewObj<DotSymbol>(),
        NewIdentifierToken(memberName2),
        NewObj<EofToken>()
    }) };
    REQUIRE(rightPostfixExpression->IsStructureReference());
    REQUIRE(rightPostfixExpression->IsValid());

    Rc<PostfixExpression> leftPostfixExpression{
        M<PostfixExpression>(rightPostfixExpression->GetChildren()[0])
    };
    REQUIRE(leftPostfixExpression->IsStructureReference());
    REQUIRE(leftPostfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(leftPostfixExpression->GetChildren()[0]) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChildren()[0]) };
    REQUIRE(objToken->GetName() == objName);

    M<DotSymbol>(leftPostfixExpression->GetChildren()[1]);

    Rc<IdentifierToken> member1{ M<IdentifierToken>(leftPostfixExpression->GetChildren()[2]) };
    REQUIRE(member1->GetName() == memberName1);

    M<DotSymbol>(rightPostfixExpression->GetChildren()[1]);

    Rc<IdentifierToken> member2{ M<IdentifierToken>(rightPostfixExpression->GetChildren()[2]) };
    REQUIRE(member2->GetName() == memberName2);
}

TEST_CASE("ExpressionParser PostfixExpression MemberAccess_MissingMemberName") {
    std::string objName{ "FooBar" };
    std::string memberName{ "Value" };
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>({
        NewIdentifierToken(objName),
        NewObj<DotSymbol>(),
        NewIdentifierToken(memberName),
        NewObj<EofToken>()
    }) };
    REQUIRE(postfixExpression->IsStructureReference());
    REQUIRE(!postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChildren()[0]) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChildren()[0]) };
    REQUIRE(objToken->GetName() == objName);

    M<DotSymbol>(postfixExpression->GetChildren()[1]);

    Rc<SyntaxNode> memberBase{ postfixExpression->GetChildren()[2] };
    REQUIRE(!memberBase);
}

TEST_CASE("ExpressionParser PostfixExpression MemberPointerAccess") {
    std::string objName{ "FooBar" };
    std::string memberName{ "Value" };
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>({
        NewIdentifierToken(objName),
        NewObj<MinusGtSymbol>(),
        NewIdentifierToken(memberName),
        NewObj<EofToken>()
    }) };
    REQUIRE(postfixExpression->IsStructureDereference());
    REQUIRE(postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChildren()[0]) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChildren()[0]) };
    REQUIRE(objToken->GetName() == objName);

    M<MinusGtSymbol>(postfixExpression->GetChildren()[1]);

    Rc<IdentifierToken> member{ M<IdentifierToken>(postfixExpression->GetChildren()[2]) };
    REQUIRE(member->GetName() == memberName);
}

TEST_CASE("ExpressionParser PostfixExpression MemberPointerAccess_Chained") {
    std::string objName{ "FooBar" };
    std::string memberName1{ "Member1" };
    std::string memberName2{ "Member2" };
    Rc<PostfixExpression> rightPostfixExpression{ Setup<PostfixExpression>({
        NewIdentifierToken(objName),
        NewObj<MinusGtSymbol>(),
        NewIdentifierToken(memberName1),
        NewObj<MinusGtSymbol>(),
        NewIdentifierToken(memberName2),
        NewObj<EofToken>()
    }) };
    REQUIRE(rightPostfixExpression->IsStructureDereference());
    REQUIRE(rightPostfixExpression->IsValid());

    Rc<PostfixExpression> leftPostfixExpression{
        M<PostfixExpression>(rightPostfixExpression->GetChildren()[0])
    };
    REQUIRE(leftPostfixExpression->IsStructureDereference());
    REQUIRE(leftPostfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(leftPostfixExpression->GetChildren()[0]) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChildren()[0]) };
    REQUIRE(objToken->GetName() == objName);

    M<MinusGtSymbol>(leftPostfixExpression->GetChildren()[1]);

    Rc<IdentifierToken> member1{ M<IdentifierToken>(leftPostfixExpression->GetChildren()[2]) };
    REQUIRE(member1->GetName() == memberName1);

    M<MinusGtSymbol>(rightPostfixExpression->GetChildren()[1]);

    Rc<IdentifierToken> member2{ M<IdentifierToken>(rightPostfixExpression->GetChildren()[2]) };
    REQUIRE(member2->GetName() == memberName2);
}

TEST_CASE("ExpressionParser PostfixExpression MemberPointerAccess_MissingMemberName") {
    std::string objName{ "FooBar" };
    std::string memberName{ "Value" };
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>({
        NewIdentifierToken(objName),
        NewObj<MinusGtSymbol>(),
        NewIdentifierToken(memberName),
        NewObj<EofToken>()
    }) };
    REQUIRE(postfixExpression->IsStructureDereference());
    REQUIRE(!postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChildren()[0]) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChildren()[0]) };
    REQUIRE(objToken->GetName() == objName);

    M<MinusGtSymbol>(postfixExpression->GetChildren()[1]);

    Rc<SyntaxNode> memberBase{ postfixExpression->GetChildren()[2] };
    REQUIRE(!memberBase);
}

TEST_CASE("ExpressionParser PostfixExpression PostIncrement") {
    std::string objName{ "FooBar" };
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>({
        NewIdentifierToken(objName),
        NewObj<PlusPlusSymbol>(),
        NewObj<EofToken>()
    }) };
    REQUIRE(postfixExpression->IsPostIncrement());
    REQUIRE(postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChildren()[0]) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChildren()[0]) };
    REQUIRE(objToken->GetName() == objName);

    M<PlusPlusSymbol>(postfixExpression->GetChildren()[1]);
}

TEST_CASE("ExpressionParser PostfixExpression PostIncrement_Chained") {
    std::string objName{ "FooBar" };
    Rc<PostfixExpression> rightPostfixExpression{ Setup<PostfixExpression>({
        NewIdentifierToken(objName),
        NewObj<PlusPlusSymbol>(),
        NewObj<PlusPlusSymbol>(),
        NewObj<EofToken>()
    }) };
    REQUIRE(rightPostfixExpression->IsPostIncrement());
    REQUIRE(rightPostfixExpression->IsValid());

    Rc<PostfixExpression> leftPostfixExpression{
        M<PostfixExpression>(rightPostfixExpression->GetChildren()[0])
    };
    REQUIRE(leftPostfixExpression->IsPostIncrement());
    REQUIRE(leftPostfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(leftPostfixExpression->GetChildren()[0]) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChildren()[0]) };
    REQUIRE(objToken->GetName() == objName);

    M<PlusPlusSymbol>(leftPostfixExpression->GetChildren()[1]);
    M<PlusPlusSymbol>(rightPostfixExpression->GetChildren()[1]);
}

TEST_CASE("ExpressionParser PostfixExpression PostDecrement") {
    std::string objName{ "FooBar" };
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>({
        NewIdentifierToken(objName),
        NewObj<MinusMinusSymbol>(),
        NewObj<EofToken>()
    }) };
    REQUIRE(postfixExpression->IsPostDecrement());
    REQUIRE(postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChildren()[0]) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChildren()[0]) };
    REQUIRE(objToken->GetName() == objName);

    M<MinusMinusSymbol>(postfixExpression->GetChildren()[1]);
}

TEST_CASE("ExpressionParser PostfixExpression PostDecrement_Chained") {
    std::string objName{ "FooBar" };
    Rc<PostfixExpression> rightPostfixExpression{ Setup<PostfixExpression>({
        NewIdentifierToken(objName),
        NewObj<MinusMinusSymbol>(),
        NewObj<MinusMinusSymbol>(),
        NewObj<EofToken>()
    }) };
    REQUIRE(rightPostfixExpression->IsPostDecrement());
    REQUIRE(rightPostfixExpression->IsValid());

    Rc<PostfixExpression> leftPostfixExpression{
        M<PostfixExpression>(rightPostfixExpression->GetChildren()[0])
    };
    REQUIRE(leftPostfixExpression->IsPostDecrement());
    REQUIRE(leftPostfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(leftPostfixExpression->GetChildren()[0]) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChildren()[0]) };
    REQUIRE(objToken->GetName() == objName);

    M<MinusMinusSymbol>(leftPostfixExpression->GetChildren()[1]);
    M<MinusMinusSymbol>(rightPostfixExpression->GetChildren()[1]);
}
