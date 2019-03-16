#include "syntax.hh"

#define O(className)          \
    className::className() {} \
    className::~className() {}
#define Sn(className)
#define Tk(className) O(className)
#include "syntax-kinds.def"
#undef Tk
#undef Sn
#undef O

#define O(className)                                          \
    Rc<Object> className::Accept(SyntaxNodeVisitor& visitor) { \
        return visitor.Visit(*this);                           \
    }                                                          \
    class className
#define Sn(className) O(className)
#define Tk(className) O(className)
#include "syntax-kinds.def"
#undef Tk
#undef Sn
#undef O

bool PrimaryExpression::IsIdentifier() const {
    return children.size() == 1 && IsSyntaxNode<IdentifierToken>(children[0]);
}
bool PrimaryExpression::IsNumericLiteral() const {
    return children.size() == 1 && IsSyntaxNode<NumericLiteralToken>(children[0]);
}
bool PrimaryExpression::IsStringLiteral() const {
    return children.size() == 1 && IsSyntaxNode<StringLiteralToken>(children[0]);
}
bool PrimaryExpression::IsParenthesizedExpression() const {
    return children.size() == 3
        && IsSyntaxNode<LParenSymbol>(children[0])
        && IsBaseOfSyntaxNode<Expression>(children[1])
        && IsSyntaxNode<RParenSymbol>(children[2]);
}
bool PrimaryExpression::IsValid() const {
    return IsIdentifier()
        || IsNumericLiteral()
        || IsStringLiteral()
        || IsParenthesizedExpression();
}

bool PostfixExpression::IsPrimaryExpression() const {
    return children.size() == 1 && IsSyntaxNode<PrimaryExpression>(children[0]);
}
bool PostfixExpression::IsArrayAccessor() const {
    return children.size() == 4
        && (IsSyntaxNode<PrimaryExpression>(children[0])
            || IsSyntaxNode<PostfixExpression>(children[0]))
        && IsSyntaxNode<LBracketSymbol>(children[1])
        && IsBaseOfSyntaxNode<Expression>(children[2])
        && IsSyntaxNode<RBracketSymbol>(children[3]);
}
bool PostfixExpression::IsFunctionCall() const {
    return false;
}
bool PostfixExpression::IsStructureReference() const {
    return children.size() == 3
        && (IsSyntaxNode<PrimaryExpression>(children[0])
            || IsSyntaxNode<PostfixExpression>(children[0]))
        && IsSyntaxNode<DotSymbol>(children[1])
        && IsSyntaxNode<IdentifierToken>(children[2]);
}
bool PostfixExpression::IsStructureDereference() const {
    return children.size() == 3
        && IsSyntaxNode<PostfixExpression>(children[0])
        && IsSyntaxNode<MinusGtSymbol>(children[1])
        && IsSyntaxNode<IdentifierToken>(children[2]);
}
bool PostfixExpression::IsPostIncrement() const {
    return children.size() == 2
        && IsSyntaxNode<PostfixExpression>(children[0])
        && IsSyntaxNode<PlusPlusSymbol>(children[1]);
}
bool PostfixExpression::IsPostDecrement() const {
    return children.size() == 2
        && IsSyntaxNode<PostfixExpression>(children[0])
        && IsSyntaxNode<MinusMinusSymbol>(children[1]);
}
bool PostfixExpression::IsValid() const {
    return IsPrimaryExpression()
        || IsArrayAccessor()
        || IsFunctionCall()
        || IsStructureReference()
        || IsStructureDereference()
        || IsPostIncrement()
        || IsPostDecrement();
}
