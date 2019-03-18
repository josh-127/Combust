#include <catch.hpp>
#include "../backtracking-lexer.hh"
#include "../code-lexer.hh"
#include "../language-parser.hh"
#include "../source.hh"
#include "../syntax.hh"
#include <vector>

template<typename T>
static inline Rc<T> M(Rc<SyntaxNode> node) {
    REQUIRE(node);
    REQUIRE(IsSyntaxNode<T>(node));
    return As<T>(node);
}

template<typename T>
static Rc<T> Setup(const std::string& source) {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", source) };
    Rc<CodeLexer> codeLexer{ NewObj<CodeLexer>(sourceFile) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(codeLexer) };

    Rc<Expression> expression{ ParseExpression(backtrackingLexer) };
    return M<T>(expression);
}

TEST_CASE("ExpressionParser PrimaryExpression EmptyFile") {
    Rc<SourceFile> sourceFile{ CreateSourceFile("", "") };
    Rc<CodeLexer> codeLexer{ NewObj<CodeLexer>(sourceFile) };
    Rc<BacktrackingLexer> backtrackingLexer{ NewObj<BacktrackingLexer>(codeLexer) };

    Rc<Expression> expression{ ParseExpression(backtrackingLexer) };
    REQUIRE(!expression);
}

TEST_CASE("ExpressionParser PrimaryExpression Identifier") {
    Rc<PrimaryExpression> expression{ Setup<PrimaryExpression>("FooBar") };
    REQUIRE(expression->IsIdentifier());
    REQUIRE(expression->IsValid());

    Rc<IdentifierToken> identifier{ M<IdentifierToken>(expression->GetChild(0)) };
    REQUIRE(identifier->GetName() == "FooBar");
}

TEST_CASE("ExpressionParser PrimaryExpression NumericLiteral") {
    Rc<PrimaryExpression> expression{ Setup<PrimaryExpression>("1234567890") };
    REQUIRE(expression->IsNumericLiteral());
    REQUIRE(expression->IsValid());

    Rc<NumericLiteralToken> identifier{ M<NumericLiteralToken>(expression->GetChild(0)) };
    REQUIRE(identifier->GetWholeValue() == "1234567890");
}

TEST_CASE("ExpressionParser PrimaryExpression StringLiteral") {
    std::string literalValue{ "The quick brown fox jumps over the lazy dog." };
    Rc<PrimaryExpression> expression{ Setup<PrimaryExpression>("\"" + literalValue + "\"") };
    REQUIRE(expression->IsStringLiteral());
    REQUIRE(expression->IsValid());

    Rc<StringLiteralToken> literalToken{ M<StringLiteralToken>(expression->GetChild(0)) };
    REQUIRE(literalToken->GetValue() == literalValue);
}

TEST_CASE("ExpressionParser PrimaryExpression ParenthesizedExpression") {
    Rc<PrimaryExpression> expression{ Setup<PrimaryExpression>("(FooBar)") };
    REQUIRE(expression->IsParenthesizedExpression());
    REQUIRE(expression->IsValid());

    M<LParenSymbol>(expression->GetChild(0));

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(expression->GetChild(1)) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    M<LParenSymbol>(expression->GetChild(0));

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChild(0)) };
    REQUIRE(objToken->GetName() == "FooBar");

    M<RParenSymbol>(expression->GetChild(2));
}

TEST_CASE("ExpressionParser PostfixExpression ArrayAccess") {
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>("FooBar[1000]") };
    REQUIRE(postfixExpression->IsArrayAccessor());
    REQUIRE(postfixExpression->IsValid());

    Rc<PrimaryExpression> primaryExpression{ M<PrimaryExpression>(postfixExpression->GetChild(0)) };
    REQUIRE(primaryExpression->IsIdentifier());
    REQUIRE(primaryExpression->IsValid());

    Rc<IdentifierToken> identifier{ M<IdentifierToken>(primaryExpression->GetChild(0)) };
    REQUIRE(identifier->GetName() == "FooBar");

    M<LBracketSymbol>(postfixExpression->GetChild(1));

    Rc<PrimaryExpression> indexValue{ M<PrimaryExpression>(postfixExpression->GetChild(2)) };
    REQUIRE(indexValue->IsNumericLiteral());
    REQUIRE(indexValue->IsValid());

    Rc<NumericLiteralToken> numericLiteral{ M<NumericLiteralToken>(indexValue->GetChild(0)) };
    REQUIRE(numericLiteral->GetWholeValue() == "1000");
}

TEST_CASE("ExpressionParser PostfixExpression ArrayAccess_Chained") {
    Rc<PostfixExpression> rightPostfixExpression{ Setup<PostfixExpression>("FooBar[1000][2000]") };
    REQUIRE(rightPostfixExpression->IsArrayAccessor());
    REQUIRE(rightPostfixExpression->IsValid());

    Rc<PostfixExpression> leftPostfixExpression{ M<PostfixExpression>(rightPostfixExpression->GetChild(0)) };
    REQUIRE(leftPostfixExpression->IsArrayAccessor());
    REQUIRE(leftPostfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(leftPostfixExpression->GetChild(0)) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChild(0)) };
    REQUIRE(objToken->GetName() == "FooBar");

    M<LBracketSymbol>(leftPostfixExpression->GetChild(1));

    Rc<PrimaryExpression> leftIndex{ M<PrimaryExpression>(leftPostfixExpression->GetChild(2)) };
    REQUIRE(leftIndex->IsNumericLiteral());
    REQUIRE(leftIndex->IsValid());

    Rc<NumericLiteralToken> leftIndexToken{ M<NumericLiteralToken>(leftIndex->GetChild(0)) };
    REQUIRE(leftIndexToken->GetWholeValue() == "1000");

    M<RBracketSymbol>(leftPostfixExpression->GetChild(3));

    M<LBracketSymbol>(rightPostfixExpression->GetChild(1));

    Rc<PrimaryExpression> rightIndex{ M<PrimaryExpression>(rightPostfixExpression->GetChild(2)) };
    REQUIRE(rightIndex->IsNumericLiteral());
    REQUIRE(rightIndex->IsValid());

    Rc<NumericLiteralToken> rightIndexToken{ M<NumericLiteralToken>(rightIndex->GetChild(0)) };
    REQUIRE(rightIndexToken->GetWholeValue() == "2000");

    M<RBracketSymbol>(rightPostfixExpression->GetChild(3));
}

TEST_CASE("ExpressionParser PostfixExpression ArrayAccess_MissingRBracket") {
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>("FooBar[1000") };
    REQUIRE(postfixExpression->IsArrayAccessor());
    REQUIRE(!postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChild(0)) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChild(0)) };
    REQUIRE(objToken->GetName() == "FooBar");

    M<LBracketSymbol>(postfixExpression->GetChild(1));

    Rc<PrimaryExpression> index{ M<PrimaryExpression>(postfixExpression->GetChild(2)) };
    REQUIRE(index->IsIdentifier());
    REQUIRE(index->IsValid());

    Rc<NumericLiteralToken> indexToken{ M<NumericLiteralToken>(index->GetChild(0)) };
    REQUIRE(indexToken->GetWholeValue() == "1000");

    Rc<SyntaxNode> rBracket{ postfixExpression->GetChild(3) };
    REQUIRE(!rBracket);
}

TEST_CASE("ExpressionParser PostfixExpression ArrayAccess_MissingFirstRBracket_Chained") {
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>("FooBar[1000[2000]") };
    REQUIRE(postfixExpression->IsArrayAccessor());
    REQUIRE(!postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChild(0)) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChild(0)) };
    REQUIRE(objToken->GetName() == "FooBar");

    M<LBracketSymbol>(postfixExpression->GetChild(1));

    Rc<PrimaryExpression> index{ M<PrimaryExpression>(postfixExpression->GetChild(2)) };
    REQUIRE(index->IsIdentifier());
    REQUIRE(index->IsValid());

    Rc<NumericLiteralToken> indexToken{ M<NumericLiteralToken>(index->GetChild(0)) };
    REQUIRE(indexToken->GetWholeValue() == "1000");

    Rc<SyntaxNode> rBracket{ postfixExpression->GetChild(3) };
    REQUIRE(!rBracket);
}

TEST_CASE("ExpressionParser PostfixExpression ArrayAccess_MissingSecondRBracket_Chained") {
    Rc<PostfixExpression> rightPostfixExpression{ Setup<PostfixExpression>("FooBar[1000][2000") };
    REQUIRE(rightPostfixExpression->IsArrayAccessor());
    REQUIRE(!rightPostfixExpression->IsValid());

    Rc<PostfixExpression> leftPostfixExpression{ M<PostfixExpression>(rightPostfixExpression->GetChild(0)) };
    REQUIRE(leftPostfixExpression->IsArrayAccessor());
    REQUIRE(leftPostfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(leftPostfixExpression->GetChild(0)) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChild(0)) };
    REQUIRE(objToken->GetName() == "FooBar");

    M<LBracketSymbol>(leftPostfixExpression->GetChild(1));

    Rc<PrimaryExpression> leftIndex{ M<PrimaryExpression>(leftPostfixExpression->GetChild(2)) };
    REQUIRE(leftIndex->IsNumericLiteral());
    REQUIRE(leftIndex->IsValid());

    Rc<NumericLiteralToken> leftIndexToken{ M<NumericLiteralToken>(leftIndex->GetChild(0)) };
    REQUIRE(leftIndexToken->GetWholeValue() == "1000");

    M<RBracketSymbol>(leftPostfixExpression->GetChild(3));

    M<LBracketSymbol>(rightPostfixExpression->GetChild(1));

    Rc<PrimaryExpression> rightIndex{ M<PrimaryExpression>(rightPostfixExpression->GetChild(2)) };
    REQUIRE(rightIndex->IsNumericLiteral());
    REQUIRE(rightIndex->IsValid());

    Rc<NumericLiteralToken> rightIndexToken{ M<NumericLiteralToken>(rightIndex->GetChild(0)) };
    REQUIRE(rightIndexToken->GetWholeValue() == "2000");

    Rc<SyntaxNode> rightRBracket{ rightPostfixExpression->GetChild(3) };
    REQUIRE(!rightRBracket);
}

TEST_CASE("ExpressionParser PostfixExpression ArrayAccess_MissingBothRBrackets_Chained") {
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>("FooBar[1000[2000") };
    REQUIRE(postfixExpression->IsArrayAccessor());
    REQUIRE(!postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChild(0)) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChild(0)) };
    REQUIRE(objToken->GetName() == "FooBar");

    M<LBracketSymbol>(postfixExpression->GetChild(1));

    Rc<PrimaryExpression> index{ M<PrimaryExpression>(postfixExpression->GetChild(2)) };
    REQUIRE(index->IsIdentifier());
    REQUIRE(index->IsValid());

    Rc<NumericLiteralToken> indexToken{ M<NumericLiteralToken>(index->GetChild(0)) };
    REQUIRE(indexToken->GetWholeValue() == "1000");

    Rc<SyntaxNode> rBracket{ postfixExpression->GetChild(3) };
    REQUIRE(!rBracket);
}

TEST_CASE("ExpressionParser PostfixExpression MemberAccess") {
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>("FooBar.Value") };
    REQUIRE(postfixExpression->IsStructureReference());
    REQUIRE(postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChild(0)) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChild(0)) };
    REQUIRE(objToken->GetName() == "FooBar");

    M<DotSymbol>(postfixExpression->GetChild(1));

    Rc<IdentifierToken> member{ M<IdentifierToken>(postfixExpression->GetChild(2)) };
    REQUIRE(member->GetName() == "Value");
}

TEST_CASE("ExpressionParser PostfixExpression MemberAccess_Chained") {
    Rc<PostfixExpression> rightPostfixExpression{ Setup<PostfixExpression>("FooBar.Member1.Member2") };
    REQUIRE(rightPostfixExpression->IsStructureReference());
    REQUIRE(rightPostfixExpression->IsValid());

    Rc<PostfixExpression> leftPostfixExpression{ M<PostfixExpression>(rightPostfixExpression->GetChild(0)) };
    REQUIRE(leftPostfixExpression->IsStructureReference());
    REQUIRE(leftPostfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(leftPostfixExpression->GetChild(0)) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChild(0)) };
    REQUIRE(objToken->GetName() == "FooBar");

    M<DotSymbol>(leftPostfixExpression->GetChild(1));

    Rc<IdentifierToken> member1{ M<IdentifierToken>(leftPostfixExpression->GetChild(2)) };
    REQUIRE(member1->GetName() == "Member1");

    M<DotSymbol>(rightPostfixExpression->GetChild(1));

    Rc<IdentifierToken> member2{ M<IdentifierToken>(rightPostfixExpression->GetChild(2)) };
    REQUIRE(member2->GetName() == "Member2");
}

TEST_CASE("ExpressionParser PostfixExpression MemberAccess_MissingMemberName") {
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>("FooBar.") };
    REQUIRE(postfixExpression->IsStructureReference());
    REQUIRE(!postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChild(0)) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChild(0)) };
    REQUIRE(objToken->GetName() == "FooBar");

    M<DotSymbol>(postfixExpression->GetChild(1));

    Rc<SyntaxNode> memberBase{ postfixExpression->GetChild(2) };
    REQUIRE(!memberBase);
}

TEST_CASE("ExpressionParser PostfixExpression MemberPointerAccess") {
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>("FooBar->Value") };
    REQUIRE(postfixExpression->IsStructureDereference());
    REQUIRE(postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChild(0)) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChild(0)) };
    REQUIRE(objToken->GetName() == "FooBar");

    M<MinusGtSymbol>(postfixExpression->GetChild(1));

    Rc<IdentifierToken> member{ M<IdentifierToken>(postfixExpression->GetChild(2)) };
    REQUIRE(member->GetName() == "Value");
}

TEST_CASE("ExpressionParser PostfixExpression MemberPointerAccess_Chained") {
    Rc<PostfixExpression> rightPostfixExpression{ Setup<PostfixExpression>("FooBar->Member1->Member2") };
    REQUIRE(rightPostfixExpression->IsStructureDereference());
    REQUIRE(rightPostfixExpression->IsValid());

    Rc<PostfixExpression> leftPostfixExpression{ M<PostfixExpression>(rightPostfixExpression->GetChild(0)) };
    REQUIRE(leftPostfixExpression->IsStructureDereference());
    REQUIRE(leftPostfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(leftPostfixExpression->GetChild(0)) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChild(0)) };
    REQUIRE(objToken->GetName() == "FooBar");

    M<MinusGtSymbol>(leftPostfixExpression->GetChild(1));

    Rc<IdentifierToken> member1{ M<IdentifierToken>(leftPostfixExpression->GetChild(2)) };
    REQUIRE(member1->GetName() == "Member1");

    M<MinusGtSymbol>(rightPostfixExpression->GetChild(1));

    Rc<IdentifierToken> member2{ M<IdentifierToken>(rightPostfixExpression->GetChild(2)) };
    REQUIRE(member2->GetName() == "Member2");
}

TEST_CASE("ExpressionParser PostfixExpression MemberPointerAccess_MissingMemberName") {
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>("FooBar->") };
    REQUIRE(postfixExpression->IsStructureDereference());
    REQUIRE(!postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChild(0)) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChild(0)) };
    REQUIRE(objToken->GetName() == "FooBar");

    M<MinusGtSymbol>(postfixExpression->GetChild(1));

    Rc<SyntaxNode> memberBase{ postfixExpression->GetChild(2) };
    REQUIRE(!memberBase);
}

TEST_CASE("ExpressionParser PostfixExpression PostIncrement") {
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>("FooBar++") };
    REQUIRE(postfixExpression->IsPostIncrement());
    REQUIRE(postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChild(0)) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChild(0)) };
    REQUIRE(objToken->GetName() == "FooBar");

    M<PlusPlusSymbol>(postfixExpression->GetChild(1));
}

TEST_CASE("ExpressionParser PostfixExpression PostIncrement_Chained") {
    Rc<PostfixExpression> rightPostfixExpression{ Setup<PostfixExpression>("FooBar++++") };
    REQUIRE(rightPostfixExpression->IsPostIncrement());
    REQUIRE(rightPostfixExpression->IsValid());

    Rc<PostfixExpression> leftPostfixExpression{ M<PostfixExpression>(rightPostfixExpression->GetChild(0)) };
    REQUIRE(leftPostfixExpression->IsPostIncrement());
    REQUIRE(leftPostfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(leftPostfixExpression->GetChild(0)) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChild(0)) };
    REQUIRE(objToken->GetName() == "FooBar");

    M<PlusPlusSymbol>(leftPostfixExpression->GetChild(1));
    M<PlusPlusSymbol>(rightPostfixExpression->GetChild(1));
}

TEST_CASE("ExpressionParser PostfixExpression PostDecrement") {
    Rc<PostfixExpression> postfixExpression{ Setup<PostfixExpression>("FooBar--") };
    REQUIRE(postfixExpression->IsPostDecrement());
    REQUIRE(postfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(postfixExpression->GetChild(0)) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChild(0)) };
    REQUIRE(objToken->GetName() == "FooBar");

    M<MinusMinusSymbol>(postfixExpression->GetChild(1));
}

TEST_CASE("ExpressionParser PostfixExpression PostDecrement_Chained") {
    Rc<PostfixExpression> rightPostfixExpression{ Setup<PostfixExpression>("FooBar----") };
    REQUIRE(rightPostfixExpression->IsPostDecrement());
    REQUIRE(rightPostfixExpression->IsValid());

    Rc<PostfixExpression> leftPostfixExpression{ M<PostfixExpression>(rightPostfixExpression->GetChild(0)) };
    REQUIRE(leftPostfixExpression->IsPostDecrement());
    REQUIRE(leftPostfixExpression->IsValid());

    Rc<PrimaryExpression> obj{ M<PrimaryExpression>(leftPostfixExpression->GetChild(0)) };
    REQUIRE(obj->IsIdentifier());
    REQUIRE(obj->IsValid());

    Rc<IdentifierToken> objToken{ M<IdentifierToken>(obj->GetChild(0)) };
    REQUIRE(objToken->GetName() == "FooBar");

    M<MinusMinusSymbol>(leftPostfixExpression->GetChild(1));
    M<MinusMinusSymbol>(rightPostfixExpression->GetChild(1));
}
